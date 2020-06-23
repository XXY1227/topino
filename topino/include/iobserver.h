#ifndef IOBSERVER_H
#define IOBSERVER_H


class IObserver {
  public:
    IObserver();
    ~IObserver();

    virtual void modelHasChanged() = 0;
};

#endif // IOBSERVER_H
