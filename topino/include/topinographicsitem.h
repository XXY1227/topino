#ifndef TOPINOGRAPHICSITEM_H
#define TOPINOGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QGraphicsObject>

class TopinoGraphicsItem : public QGraphicsObject {
    Q_OBJECT

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

    /* Updates the scaling of the item */
    virtual void updateScale() = 0;

    /* Returns and sets the ID of the item, used for organizing */
    int getItemid() const;
    void setItemid(int value);

    /* Returns and sets the scaling of the item; used so that the item
     * always draws in a reasonable size on the image */
    double getScaling() const;
    void setScaling(double value);

  protected:
    /* If the item properties are changed, propagate this information upwards */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  signals:
    void itemHasChanged(const TopinoGraphicsItem *item);

  private:
    int itemid = 0;
    double scaling = 1.0f;
};

#endif // TOPINOGRAPHICSITEM_H
