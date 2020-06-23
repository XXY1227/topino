#include "include/topinodocument.h"

#include <algorithm>

TopinoDocument::TopinoDocument() {
}

TopinoDocument::~TopinoDocument() {
}

void TopinoDocument::addObserver(IObserver *observer) {
    if (std::find(observers.begin(), observers.end(), observer) == observers.end())
        observers.push_back(observer);
}

void TopinoDocument::removeObserver(IObserver *observer) {
    auto item = std::find(observers.begin(), observers.end(), observer);

    if (item != observers.end())
        observers.erase(item);
}

void TopinoDocument::notifyAllObserver() {
    for (auto observer : observers)
        observer->modelHasChanged();
}

void TopinoDocument::modify() {
    changed = true;
    notifyAllObserver();
}
