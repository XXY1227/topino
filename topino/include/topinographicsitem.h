#ifndef TOPINOGRAPHICSITEM_H
#define TOPINOGRAPHICSITEM_H

#include <QGraphicsItem>

class TopinoGraphicsItem : public QGraphicsItem {
  public:
    TopinoGraphicsItem(int newitemid, QGraphicsItem *parent = nullptr);
    ~TopinoGraphicsItem();

    /* These enums define what type of item the user can interact with
     * and which property page is shown in the user interface, etc. */
    enum itemtype {
        nonspecific = 0,
        ruler = 1,
        inlet = 2,
        count = 3
    };

    /* Returns the type of item */
    virtual itemtype getItemType() const = 0;

    /* Returns and sets the ID of the item, used for organizing */
    int getItemid() const;
    void setItemid(int value);

  private:
    int itemid = 0;
};

#endif // TOPINOGRAPHICSITEM_H
