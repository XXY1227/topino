#include "include/topinographicsitem.h"

TopinoGraphicsItem::TopinoGraphicsItem(int newitemid, QGraphicsItem* parent) : QGraphicsObject(parent) {
    /* Set item ID */
    itemid = newitemid;

    /* Prepare selection pen; important bit here is the cosmetic value that stops scaling the pen width
     * from scaling with item/scene/viewport etc. */
    penSelection = QPen(Qt::white, 1, Qt::DashLine);
    penSelection.setCosmetic(true);
}

TopinoGraphicsItem::~TopinoGraphicsItem() {
}

int TopinoGraphicsItem::getItemid() const {
    return itemid;
}

void TopinoGraphicsItem::setItemid(int value) {
    itemid = value;
}

double TopinoGraphicsItem::getScaling() const {
    return scaling;
}

void TopinoGraphicsItem::setScaling(double value) {
    scaling = value;
    updateScale();
}

QVariant TopinoGraphicsItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    /* Item has changed, send a signal */
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit itemPosChanged(this);
    }

    /* Otherwise proceed with the normal handling of this event handler */
    return QGraphicsObject::itemChange(change, value);
}
