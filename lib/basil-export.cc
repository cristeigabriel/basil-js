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

double function_test(double test) {

  return test+50.5;
}

using namespace basil;
namespace basiljs {
Napi::Number WRAPPER(const Napi::CallbackInfo& info) {
    // environment
    Napi::Env env = info.Env();

    // check if the arguments provided are valid
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "GENERIC ERROR MESSAGE HERE:").ThrowAsJavaScriptException();
    }

    // create a "variable"
    // object or w/e
    Napi::Number addressToCall = info[0].As<Napi::Number>();

    // do something with that object we created
    Napi::Number returnValue = Napi::Number::New(env, function_test(addressToCall.DoubleValue()));

    return returnValue;
}
}  // namespace basiljs

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
    exports.Set("function_test", Napi::Function::New(env, basiljs::WRAPPER));

    return exports;
}

NODE_API_MODULE(basil, Initialize);