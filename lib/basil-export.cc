/**
 * @file basil-export.cc
 * @author otvv & Cristei Gabriel-Marian (cristei.g772@gmail.com)
 * @brief basil Bindings to Node JS.
 * @version 0.1
 * @date 2021-09-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "../basil/basil.hh"
#include <memory>
#include <type_traits>
#include <napi.h>

/**
 * @todo: Rewrite this eventually? This should use the following:
 * 
 * https://github.com/nodejs/node-addon-api/blob/main/doc/object_wrap.md
 * 
 * But I haven't had any success using this
 * 
 * - cristeigabriel 9/10/21
 * 
 */

/**
 * @brief This implies only one instance, therefore, basil is exporting a 'set' method.
 * A base import implies a context.
 * 
 * Lifetime is of global scope.
 * 
 */
inline std::unique_ptr<basil::ctx> _ctx;

/**
 * @brief Connected to _ctx
 * CPP and JS managed methods 
 * 
 */
namespace wrappers {
namespace cpp {
    template<typename T>
    T read(uintptr_t at) {
        const auto read = _ctx->read_memory<T>(at);
        if (read.has_value()) {
            auto [bytes, bytes_read] = read.value();

            if (bytes_read != sizeof(T)) {
                throw std::runtime_error(__FUNCTION__ ": Failed! Partial read.");
                goto as_is;
            }

            return bytes;
        } else {
            throw std::runtime_error(__FUNCTION__ ": Failed! Possible unreadable section.");
            goto as_is;
        }

    as_is:
        return T {};
    }
}  // namespace cpp
namespace js {
    Napi::Function capture_all_modules(const Napi::CallbackInfo& info) {
        const Napi::Env& env = info.Env();
        try {
            _ctx->capture_all_modules();
        } catch (const std::exception& err) {
            Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
            return {};
        }

        return {};
    }

    Napi::Value get_module(const Napi::CallbackInfo& info) {
        const Napi::Env& env = info.Env();
        Napi::Object ret     = Napi::Object::New(env);
        try {
            if (info.Length() != 1) {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given enough arguments.").ThrowAsJavaScriptException();
            }

            if (!info[0].IsString()) {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given a module.").ThrowAsJavaScriptException();
            }

            const auto module = _ctx->capture_module(info[0].ToString().Utf8Value());
            if (module.has_value()) {
                auto [start, size] = module.value();
                ret.Set("start", Napi::Number::New(env, (uintptr_t)(start)));
                ret.Set("size", Napi::Number::New(env, size));
            }
        } catch (const std::exception& err) {
            Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
            return {};
        }

        return ret;
    }

    template<typename T>
    Napi::Value read(const Napi::CallbackInfo& info) {
        const Napi::Env& env = info.Env();
        try {
            bool second  = info.Length() == 2;
            uintptr_t at = 0;

            if (info.Length() == 1 || second) {
                if (!info[0].IsNumber()) {
                    Napi::TypeError::New(env, __FUNCTION__ ": wasn't given an address.").ThrowAsJavaScriptException();
                }
            } else {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given enough arguments.").ThrowAsJavaScriptException();
            }

            if (second) {
                if (!info[1].IsString()) {
                    Napi::TypeError::New(env, __FUNCTION__ ": wasn't given a valid string.").ThrowAsJavaScriptException();
                } else {
                    const auto module = _ctx->capture_module(info[1].ToString().Utf8Value());
                    if (module.has_value()) {
                        auto [start, size] = module.value();
                        at                 = (uintptr_t)(start);
                    } else {
                        throw std::runtime_error(__FUNCTION__ ": Module isn't present.");
                    }
                }
            }

            return Napi::Number::New(env, cpp::read<T>(info[0].ToNumber().Uint32Value() + at));
        }

        catch (const std::exception& err) {
            Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
            return {};
        }

        return {};
    }

    Napi::Value pattern_scan(const Napi::CallbackInfo& info) {
        const Napi::Env& env = info.Env();
        try {
            if (info.Length() != 2) {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given enough arguments.").ThrowAsJavaScriptException();
            }

            if (!info[0].IsArray()) {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given an array.").ThrowAsJavaScriptException();
            }

            if (!info[1].IsString()) {
                Napi::TypeError::New(env, __FUNCTION__ ": wasn't given a valid string.").ThrowAsJavaScriptException();
            }

            const auto module = _ctx->capture_module(info[1].ToString().Utf8Value());
            if (module.has_value()) {
                auto [start, size] = module.value();
                const auto pattern = info[0].As<Napi::ArrayBuffer>();
                const auto ptr     = _ctx->pattern_scan_module(info[1].ToString().Utf8Value(), pattern.Data(), pattern.ByteLength());
                if (ptr.has_value()) {
                    return Napi::Number::New(env, ptr.value());
                } else {
                    throw std::runtime_error(__FUNCTION__ ": Pattern scan didn't succeed.");
                }
            } else {
                throw std::runtime_error(__FUNCTION__ ": Module isn't present.");
            }
        } catch (const std::exception& err) {
            Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
            return {};
        }

        return {};
    }

#define MAKE_TYPE(x) \
    inline Napi::Value read_##x(const Napi::CallbackInfo& info) { \
        return read<##x>(info); \
    }

// inline methods for binding to napi
#define EXECUTE_ALL_API_TYPES(macro) \
    macro(int8_t); \
    macro(int16_t); \
    macro(int32_t); \
    macro(uint8_t); \
    macro(uint16_t); \
    macro(uint32_t); \
    macro(float);

    EXECUTE_ALL_API_TYPES(MAKE_TYPE);
}  // namespace js
}  // namespace wrappers

namespace basiljs {
// sets context, returns it in JavaScript.
Napi::Object setter_getter(const Napi::CallbackInfo& info) {
    const Napi::Env& env = info.Env();
    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Basil setter wasn't given enough arguments.").ThrowAsJavaScriptException();
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Basil setter wasn't given a string argument. The setter argument must look like \"process_name.exe\".").ThrowAsJavaScriptException();
    }

    Napi::Object ret = Napi::Object::New(env);
    try {
        // set instance to handle passed process
        _ctx = std::make_unique<basil::ctx>(info[0].ToString());

        // field name from basil::ctx
        ret.Set("name", Napi::String::New(env, _ctx->get_name()));

        // managed field pid from basil::ctx
        const auto pid = _ctx->get_pid();
        if (pid.has_value()) {
            ret.Set("pid", Napi::Number::New(env, pid.value()));
        } else {
            throw std::runtime_error("UB: PID wasn't set.");
        }

        // utility functions
        ret.Set("get_module", Napi::Function::New(env, wrappers::js::get_module));

// type functions
// utility macro to run down all types
#define EXPORT_TYPE(x) ret.Set("read_" #x, Napi::Function::New(env, wrappers::js::read_##x))
        EXECUTE_ALL_API_TYPES(EXPORT_TYPE);
    } catch (const std::exception& err) {
        Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
        goto as_is;
    }

as_is:
    return ret;
}
}  // namespace basiljs

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    exports.Set("set", Napi::Function::New(env, basiljs::setter_getter));

    return exports;
}

NODE_API_MODULE(basil, Initialize);
