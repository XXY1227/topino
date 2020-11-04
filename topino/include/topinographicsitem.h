#ifndef TOPINOGRAPHICSITEM_H
#define TOPINOGRAPHICSITEM_H

#include <QPen>
#include <QGraphicsItem>
#include <QGraphicsObject>

class TopinoGraphicsItem : public QGraphicsObject {
    Q_OBJECT

  public:
    TopinoGraphicsItem(int newitemid, QGraphicsItem *parent = nullptr);
    virtual ~TopinoGraphicsItem();

    /* These enums define what type of item the user can interact with
     * and which property page is shown in the user interface, etc. */
    enum itemtype {
        nonspecific = 0,
        image = 1,
        ruler = 2,
        inlet = 3,
        count = 4
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

    /* These methods create data representations of the item in various
     * formats (and allow converting it back, too!) */
    virtual QString toString() const = 0;
    virtual void fromString(const QString &value) = 0;

  protected:
    /* Pen used to draw the selection line; it should be the same for all items and
     * purely cosmetic (not scaling at all). */
    QPen penSelection;

    /* If the item properties are changed, propagate this information upwards */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

  signals:
    void itemPosChanged(const TopinoGraphicsItem *item);
    void itemDataChanged(const TopinoGraphicsItem *item);

  private:
    /* Given item ID that links the graphical representation to the respective data item
     * in the model */
    int itemid = 0;

    /* Scaling factor used for scaling the item dimensions reasonably on the image */
    double scaling = 1.0f;    
};

#endif // TOPINOGRAPHICSITEM_H
