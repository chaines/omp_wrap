#include <napi.h>
#include <iostream>
#include "OMPEval/omp/EquityCalculator.h"
#include "OMPEval/omp/CardRange.h"
#include "OMPEval/omp/HandEvaluator.h"
#include "OMPEval/omp/Hand.h"

// My C++ skills are subpar to say the least
// I can't figure out how to call a Napi::ThreadSafeFunction within EquityCalculator,
// meaning the best way to return partial results is to change EquityCalculator to stop
// at a set time interval and provide a continue() method to fetch the next (timeInterval)
// of results until all results are returned.
//
// For now, we're just calling the callback once, at the end, synchronously.

Napi::Value CompareRobust(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Value params = info[0];
    if(params.IsEmpty() || params.IsNull() || params.IsUndefined() || !params.IsObject())
        return Napi::Boolean::New(env, false);
    Napi::Object args = params.As<Napi::Object>();
    std::vector<omp::CardRange> ranges;
    Napi::Array hands = args.Get("handRanges").As<Napi::Array>();
    for(int i = 0; i < hands.Length(); ++i) {
        //We have to explicitely cast to string, otherwise it prints a memory location
        ranges.push_back(omp::CardRange((std::string) hands.Get(i).ToString()));
    }
    std::string boardCards, deadCards;
    bool enumerateAll;
    Napi::Function callback;
    double stdevTarget, updateInterval;
    uint64_t bcMask, dcMask;
    if(args.Get("boardCards").IsString()) {
        boardCards = args.Get("boardCards").ToString();
        bcMask = omp::CardRange::getCardMask(boardCards);
    }
    else {
        boardCards = "";
        bcMask = 0;
    }
    if(args.Get("deadCards").IsString()) {
        deadCards = args.Get("deadCards").ToString();
        dcMask = omp::CardRange::getCardMask(deadCards);
    } else {
        deadCards = "";
        dcMask = 0;
    }
    enumerateAll = args.Get("enumerate").IsBoolean() ? (bool) args.Get("enumerate").ToBoolean() : false;
    if(args.Get("callback").IsFunction()) callback = args.Get("callback").As<Napi::Function>();
    stdevTarget = args.Get("stdevTarget").IsNumber() ? (double) args.Get("stdevTarget").ToNumber() : 5e-5;
    updateInterval = args.Get("updateInterval").IsNumber() ? (double) args.Get("updateInterval").ToNumber() : 0.2;

    omp::EquityCalculator calc;
    calc.start(ranges, bcMask, dcMask, enumerateAll, stdevTarget, nullptr, updateInterval);
    calc.wait();
    omp::EquityCalculator::Results results = calc.getResults();

    Napi::Object r = Napi::Object::New(env);
    Napi::Number players = Napi::Number::New(env, results.players);
    Napi::Array equities = Napi::Array::New(env);
    Napi::Array wins = Napi::Array::New(env);
    double ties = 0;
    Napi::Number count = Napi::Number::New(env, results.hands);
    Napi::Number time = Napi::Number::New(env, results.time);
    Napi::Boolean exhaustive = Napi::Boolean::New(env, results.enumerateAll);
    Napi::Boolean finished = Napi::Boolean::New(env, results.finished);

    for(int i = 0; i < results.players; i++) {
        equities.Set(i, Napi::Number::New(env, results.equity[i]));
        wins.Set(i, Napi::Number::New(env, results.wins[i]));
        ties += results.ties[i];
    }

    r.Set("players", players);
    r.Set("equities", equities);
    r.Set("wins", wins);
    r.Set("ties", Napi::Number::New(env, ties));
    r.Set("count", count);
    r.Set("time", time);
    r.Set("exhaustive", exhaustive);
    r.Set("finished", finished);

    if(!callback.IsEmpty())
        callback.Call({r});
    
    return r;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {

    exports.Set(
        Napi::String::New(env, "evaluateHands"),
        Napi::Function::New(env, CompareRobust)
    );
    return exports;
}

NODE_API_MODULE(jsomp, Init)