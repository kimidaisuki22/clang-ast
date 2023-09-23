#pragma once
#include "reflection.hpp"
namespace creflec {
constexpr auto set_json_field = [](auto &json,auto && name, auto && elem){

};
constexpr auto to_json = [](auto &&elem, auto &json) {
  for_each_member(elem,
                  [&json](auto value, auto &type, auto &name, auto index) {
                    (void)type;
                    (void)index;
                    if constexpr (requires { json[name] = value; }) {
                      json[name] = value;
                    } else if constexpr (requires { to_string(value); }) {
                      json[name] = to_string(value);
                    } else if constexpr( requires { begin(value); end(value); }){
                        for(auto& v: value){
                            json[name].push_back(v);
                        }
                    }else {
                        json[name] = {}; // set it to null.
                        // or just let compile fails.
                    }
                  });
};
}