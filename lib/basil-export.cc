/*
*/

#include "../basil/basil.hh"

#include <napi.h>

using namespace basil;

namespace basiljs
{

  //
  // go nuts.
  //
  Napi::Number WRAPPER(const Napi::CallbackInfo &info) {

    // environment
    Napi::Env env = info.Env();

    // check if the arguments provided are valid
    if (_info.Length() < 1 || !_info[0].IsNumber())
    {
      Napi::TypeError::New(env, "GENERIC ERROR MESSAGE HERE:").ThrowAsJavaScriptException();
    }

    // create a "variable"
    // object or w/e
    Napi::Number addressToCall = info[0].As<Napi::Number>();

    // do something with that object we created
    Napi::Number returnValue = Napi::Number::New(env, function_from_basil_cpp_lib(addressToCall.Int64Value()));

    return returnValue;
  }
}

Napi::Object Initialize(Napi::Env env, Napi::Object exports)
{
  exports.Set("FUNCTION_NAME (this will be the js function name)", Napi::Function::New(_env, WRAPPER));

  return exports;
}

NODE_API_MODULE(basil, Initialize);