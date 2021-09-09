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
#include <napi.h>

namespace basiljs {
Napi::Object basil_ctor(const Napi::CallbackInfo& info) {
    const auto& env = info.Env();
    if (info.Length() != 1) {
        Napi::TypeError::New(env, "Constructor wasn't given either enough arguments.").ThrowAsJavaScriptException();
    }

    if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Constructor wasn't given a string argument. The constructor argument must look like \"process_name.exe\".").ThrowAsJavaScriptException();
    }

    Napi::Object ret = Napi::Object::New(env);
    try {
        basil::ctx ctx(info[0].ToString());
        ret.Set("name", Napi::String::New(env, ctx.get_name()));
    } catch (const std::exception& err) {
        Napi::TypeError::New(env, err.what()).ThrowAsJavaScriptException();
        goto as_is;
    }

as_is:
    return ret;
}
}  // namespace basiljs

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    exports.Set("new", Napi::Function::New(env, basiljs::basil_ctor));

    return exports;
}

NODE_API_MODULE(basil, Initialize);