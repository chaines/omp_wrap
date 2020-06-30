#include <napi.h>
#include <iostream>
#include "OMPEval/omp/EquityCalculator.h"
#include "OMPEval/omp/CardRange.h"
#include "OMPEval/omp/HandEvaluator.h"
#include "OMPEval/omp/Hand.h"

Napi::Value CompareMultiple(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::vector<omp::CardRange> ranges;
    Napi::Array hands = info[0].As<Napi::Array>();
    const int len = hands.Length();
    for(int i = 0; i < len; i++) {
        ranges.push_back(omp::CardRange((std::string) hands.Get(i).ToString()));
    }
    Napi::Value param2 = info[1];
    uint64_t board = 0;
    if(!(param2.IsEmpty() && param2.IsNull() && param2.IsUndefined())) {
        if(param2.IsArray()) {
            Napi::Array boardArr = param2.As<Napi::Array>();
            std::string boardString;
            for(int i = 0; i < boardArr.Length(); i++) {
                boardString += (std::string) boardArr.Get(i).ToString();
            }
            board = omp::CardRange::getCardMask(boardString);
        } else if(param2.IsString()) {
            board = omp::CardRange::getCardMask((std::string) param2.ToString());
        }
    }

    omp::EquityCalculator calc;
    calc.start(ranges, board, 0, true, 0, nullptr, 0.1, 0);
    calc.wait();
    omp::EquityCalculator::Results results = calc.getResults();

    Napi::Object nResults = Napi::Object::New(env);
    Napi::Number players = Napi::Number::New(env, results.players);
    Napi::Array equities = Napi::Array::New(env);
    Napi::Array wins = Napi::Array::New(env);
    double ties = 0;
    Napi::Number count = Napi::Number::New(env, results.hands);
    Napi::Number time = Napi::Number::New(env, results.time);

    for(int i = 0; i < results.players; i++) {
        equities.Set(i, Napi::Number::New(env, results.equity[i]));
        wins.Set(i, Napi::Number::New(env, results.wins[i]));
        ties += results.ties[i];
    }

    nResults.Set("players", players);
    nResults.Set("equities", equities);
    nResults.Set("wins", wins);
    nResults.Set("ties", Napi::Number::New(env, ties));
    nResults.Set("count", count);
    nResults.Set("time", time);
    nResults.Set("exhaustive", Napi::Boolean::New(env, results.enumerateAll));

    return nResults;
}

Napi::Value CompareHoldemPreflop(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if(info.Length() != 2) {
        return env.Null();
    }

    std::string h1s = (std::string) info[0].ToString();
    std::string h2s = (std::string) info[1].ToString();
    omp::CardRange h1(h1s);
    omp::CardRange h2(h2s);
    omp::EquityCalculator calc;

    calc.start({h1, h2}, 0, 0, true, 0, nullptr, 0.1, 0);
    calc.wait();
    omp::EquityCalculator::Results results = calc.getResults();

    Napi::Object napiResults = Napi::Object::New(env);
    Napi::Number players = Napi::Number::New(env,results.players);
    Napi::Number h1equity = Napi::Number::New(env, results.equity[0]);
    Napi::Number h2equity = Napi::Number::New(env, results.equity[1]);
    Napi::Number numHands = Napi::Number::New(env, results.evaluatedPreflopCombos);


    napiResults.Set("players", players);
    napiResults.Set("h1equity", h1equity);
    napiResults.Set("h1wins", Napi::Number::New(env, results.wins[0]));
    napiResults.Set("h2equity", h2equity);
    napiResults.Set("h2wins", Napi::Number::New(env, results.wins[1]));
    napiResults.Set("ties", Napi::Number::New(env, results.ties[0] + results.ties[1]));
    napiResults.Set("numHands", numHands);
    napiResults.Set("count", Napi::Number::New(env, results.hands));
    napiResults.Set("time", Napi::Number::New(env, results.time));
    napiResults.Set("exhaustive", Napi::Boolean::New(env, results.enumerateAll));

    return napiResults;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(
        Napi::String::New(env, "compare2"),
        Napi::Function::New(env, CompareHoldemPreflop)
    );

    exports.Set(
        Napi::String::New(env, "compareAny"),
        Napi::Function::New(env, CompareMultiple)
    );
    return exports;
}

NODE_API_MODULE(jsomp, Init)