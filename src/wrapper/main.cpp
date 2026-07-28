
#include <valhalla/worker.h>
#include "main.h"
#include "valhalla_actor.h"

#ifdef __ANDROID__
// The Android JNI interface uses a different function signature.
#include <jni.h>

extern "C"
JNIEXPORT jstring

JNICALL
Java_com_valhalla_valhalla_ValhallaKotlin_route(JNIEnv *env,
                                                jobject thiz,
                                                jstring jRequest,
                                                jstring jConfigPath) {
    
    const char *request = env->GetStringUTFChars(jRequest, 0);
    const char *config_path = env->GetStringUTFChars(jConfigPath, 0);

    std::string result;
    try {
        // TODO: Android currently creates a new actor every time. Optimize to be like iOS later.
        ValhallaActor valhallaActor(config_path);
        result = valhallaActor.route(request);
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] route valhalla_exception: %s\n", err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] route std::exception: %s\n", err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] route unknown exception");
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    env->ReleaseStringUTFChars(jRequest, request);
    env->ReleaseStringUTFChars(jConfigPath, config_path);

    return env->NewStringUTF(result.c_str());
}

#elif __APPLE__
void* create_valhalla_actor(const char *config_path, ValhallaMobileHttpClient* http_client) {
    return new ValhallaActor(config_path, http_client);
}

void delete_valhalla_actor(void* actor) {
    delete ((ValhallaActor*) actor);
}

// Every action reports failure the same way: a JSON object carrying
// Valhalla's own error code and message, so the caller can decode one shape
// whichever action it asked for.
static std::string invoke_action(const char *action,
                                 const std::function<std::string()>& work) {
    std::string result;
    try {
        result = work();
    } catch (const valhalla::valhalla_exception_t &err) {
        printf("[ValhallaActor] %s valhalla_exception: %s\n", action, err.what());
        std::string code = std::to_string(err.code);
        std::string message = err.message.c_str();

        result = "{\"code\":" + code + ",\"message\":\"" + message + "\"}";
    } catch (const std::exception &err) {
        printf("[ValhallaActor] %s std::exception: %s\n", action, err.what());
        result = "{\"code\":-1,\"message\":\"" + std::string(err.what()) + "\"}";
    } catch (...) {
        printf("[ValhallaActor] %s unknown exception", action);
        result = "{\"code\":-1,\"message\":\"unknown exception\"}";
    }

    return result;
}

std::string route(const char *request, void* actor) {
    return invoke_action("route", [request, actor]() {
        return ((ValhallaActor*) actor)->route(request);
    });
}

std::string trace_route(const char *request, void* actor) {
    return invoke_action("trace_route", [request, actor]() {
        return ((ValhallaActor*) actor)->trace_route(request);
    });
}

std::string trace_attributes(const char *request, void* actor) {
    return invoke_action("trace_attributes", [request, actor]() {
        return ((ValhallaActor*) actor)->trace_attributes(request);
    });
}

std::string height(const char *request, void* actor) {
    return invoke_action("height", [request, actor]() {
        return ((ValhallaActor*) actor)->height(request);
    });
}
#endif
