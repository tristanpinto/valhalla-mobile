// Local benchmark driver: times route, trace_attributes, and height against
// the Andorra fixture. Built only with -DBENCH_DRIVER=ON; never shipped.
#include "valhalla_actor.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

namespace {

double median_ms(std::vector<double>& samples) {
    std::sort(samples.begin(), samples.end());
    return samples[samples.size() / 2];
}

template <typename Fn>
void run(const char* name, int iterations, Fn&& fn) {
    std::vector<double> samples;
    samples.reserve(iterations);
    for (int i = 0; i < iterations; i++) {
        const auto start = std::chrono::steady_clock::now();
        const std::string result = fn();
        const auto end = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        if (result.find("\"code\"") != std::string::npos &&
            result.find("trip") == std::string::npos &&
            result.find("height") == std::string::npos &&
            result.find("edges") == std::string::npos) {
            printf("%s FAILED: %s\n", name, result.substr(0, 200).c_str());
            return;
        }
    }
    printf("%-18s median %8.3f ms over %d runs\n", name, median_ms(samples), iterations);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: bench <config.json> [iterations]\n");
        return 1;
    }
    const int iterations = argc > 2 ? atoi(argv[2]) : 50;

    ValhallaActor actor(argv[1]);

    const std::string route_request =
        R"({"locations":[{"lat":42.5063,"lon":1.5218},{"lat":42.5086,"lon":1.5394}],"costing":"bicycle"})";
    const std::string long_route_request =
        R"({"locations":[{"lat":42.4645,"lon":1.4913},{"lat":42.5449,"lon":1.7331}],"costing":"auto"})";
    const std::string height_request =
        R"({"shape":[{"lat":42.5063,"lon":1.5218},{"lat":42.5074,"lon":1.5301},{"lat":42.5086,"lon":1.5394}]})";

    // A trace built from a real route shape, so map matching does real work.
    const std::string route_response = actor.route(route_request);
    const std::string::size_type shape_pos = route_response.find("\"shape\":\"");
    if (shape_pos == std::string::npos) {
        printf("could not extract a shape from: %s\n", route_response.substr(0, 200).c_str());
        return 1;
    }
    const auto shape_start = shape_pos + 9;
    const auto shape_end = route_response.find('"', shape_start);
    std::string shape = route_response.substr(shape_start, shape_end - shape_start);
    // The shape is JSON-escaped; unescape backslashes for reuse inside new JSON.
    const std::string trace_request =
        std::string(R"({"encoded_polyline":")") + shape +
        R"(","costing":"auto","shape_match":"map_snap"})";

    run("route/short", iterations, [&] { return actor.route(route_request); });
    run("route/long", iterations, [&] { return actor.route(long_route_request); });
    run("trace_attributes", iterations, [&] { return actor.trace_attributes(trace_request); });
    run("height", iterations, [&] { return actor.height(height_request); });
    return 0;
}
