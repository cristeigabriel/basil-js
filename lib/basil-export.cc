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
 */
inline std::shared_ptr<basil::ctx> _ctx;

/**
 * @brief Connected to _ctx
 * CPP and JS managed methods 
 * 
 */
namespace wrappers {
namespace cpp {
    template<typename T>
    T read(uintptr_t at) {
        static_assert(std::is_integral_v<T>);

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
    template<typename T>
    Napi::Value read(const Napi::CallbackInfo& info) {
        const Napi::Env& env = info.Env();
        try {
            if (info.Length() != 1) {
                Napi::TypeError::New(env, "Method " __FUNCTION__ " wasn't given either enough arguments.").ThrowAsJavaScriptException();
            }

            if (!info[0].IsNumber()) {
                Napi::TypeError::New(env, "Method " __FUNCTION__ " wasn't given an address.").ThrowAsJavaScriptException();
            }

            return Napi::Number::New(env, cpp::read<T>(info[0].ToNumber().Uint32Value()));
        } catch (const std::exception& err) {
            Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
            return {};
        }
    }

    // inline methods for binding to napi
    inline Napi::Value read_uint8(const Napi::CallbackInfo& info) {
        return read<uint8_t>(info);
    }
}  // namespace js
}  // namespace wrappers

namespace basiljs {
// Sets context, returns it in JavaScript.
Napi::Object setter_getter(const Napi::CallbackInfo& info) {
    const Napi::Env& env = info.Env();
    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Basil setter wasn't given either enough arguments.").ThrowAsJavaScriptException();
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Basil setter wasn't given a string argument. The setter argument must look like \"process_name.exe\".").ThrowAsJavaScriptException();
    }

    Napi::Object ret = Napi::Object::New(env);
    try {
        // set instance to handle passed process
        _ctx = std::make_shared<basil::ctx>(info[0].ToString());

        // field name from basil::ctx
        ret.Set("name", Napi::String::New(env, _ctx->get_name()));

        // managed field pid from basil::ctx
        const auto pid = _ctx->get_pid();
        if (pid.has_value()) {
            ret.Set("pid", Napi::Number::New(env, pid.value()));
        } else {
            throw std::runtime_error("UB: PID wasn't set.");
        }

        // read uint8_t
        ret.Set("read_uint8", Napi::Function::New(env, wrappers::js::read_uint8));
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