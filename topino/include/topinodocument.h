#ifndef TOPINODOCUMENT_H
#define TOPINODOCUMENT_H

#include <include/iobserver.h>

#include <vector>

class TopinoDocument
{
  public:
    TopinoDocument();
    ~TopinoDocument();

    void addObserver(IObserver *observer);
    void removeObserver(IObserver *observer);
    void notifyAllObserver();

    bool hasChanged() { return changed; }
    void modify();

  private:
      bool changed = false;

      std::vector<IObserver*> observers;
};

#endif // TOPINODOCUMENT_H
