#include "include/topinodata.h"

TopinoData::TopinoData() {
}

TopinoData::~TopinoData() {

}

QImage TopinoData::getImage() const {
    return image;
}

void TopinoData::setImage(const QImage& value) {
    /* Save image */
    image = value;

    /* Recreate coordinate system with some standard values */
    createCoordinateSystem();
}

TopinoData::ParsingError TopinoData::loadObject(QXmlStreamReader& xml) {
    /* Central function that loads the respective object depending on the name of the current
     * XML element; this function gets usually called by the document object */
    if (xml.name() == "sourceImage") {
        return loadImageObject(xml);
    } else if (xml.name() == "coordinateSystem") {
        return loadCoordinateObject(xml);
    }

    /* Ignored an element */
    return ParsingError::IgnoredElement;
}

TopinoData::ParsingError TopinoData::loadImageObject(QXmlStreamReader& xml) {
    /* Read all elements of an image object and fill in the respective members; the data of
     * source image is saved as base64 encoded PNG */
    while (xml.readNextStartElement()) {
        qDebug("Found element %s in image object...", xml.name().toString().toStdString().c_str());

        if(xml.name() == "data") {
            QString text = xml.readElementText();
            QByteArray bytes;
            bytes.append(text);
            bytes = QByteArray::fromBase64(bytes);
            image = QImage::fromData(bytes, "PNG");

            if (image.isNull())
                return ParsingError::CouldNotLoadImage;
        } else {
            xml.skipCurrentElement();
        }
    }

    /* No parsing error while loading the image */
    return ParsingError::NoFailure;
}

void TopinoData::saveImageObject(QXmlStreamWriter& xml) {
    /* Saves the image object and some data about it (original file name, etc) in <sourceImage> */
    xml.writeStartElement("sourceImage");

    /* TODO: source file name, etc */

    /* Actual image is converted (through a QBuffer) to a base64 encoded PNG and then written
     * into the XML as simple text element; makes it easier to edit this file with XML and
     * text editors outside of Topino */
    QByteArray bytes;
    QBuffer buffer(&bytes);
    image.save(&buffer, "PNG");
    xml.writeTextElement("data", bytes.toBase64());

    xml.writeEndElement();
}

TopinoData::ParsingError TopinoData::loadCoordinateObject(QXmlStreamReader& xml) {
    /* Read all elements of the coordinate object and fill in the respective members */
    while (xml.readNextStartElement()) {
        /* Read content of element and parse it */
        QString content = xml.readElementText().toLower();

        qDebug("Found element %s in coordinate object: %s.",
               xml.name().toString().toStdString().c_str(),
               content.toStdString().c_str());

        if(xml.name() == "originX") {
            origin.setX(content.toDouble());
        } else if (xml.name() == "originY") {
            origin.setY(content.toDouble());
        } else if (xml.name() == "neutralAngle") {
            neutralAngle = content.toInt();
        } else if (xml.name() == "counterClockwise") {
            counterClockwise = (content.compare("true") == 0);
        } else {
            xml.skipCurrentElement();
        }
    }

    /* No parsing error while loading the image */
    return ParsingError::NoFailure;
}

void TopinoData::saveCoordinateObject(QXmlStreamWriter& xml) {
    /* Save the coordinate system properties as own object */
    xml.writeStartElement("coordinateSystem");

    xml.writeTextElement("originX", QString::number(origin.x()));
    xml.writeTextElement("originY", QString::number(origin.y()));
    xml.writeTextElement("neutralAngle", QString::number(neutralAngle));
    xml.writeTextElement("counterClockwise", counterClockwise ? "true" : "false");
}

void TopinoData::createCoordinateSystem() {
    /* Only if image is set */
    if (image.isNull())
        return;

    /* Standard origin is bottom middle point */
    origin = QPointF(image.width() / 2.0, image.height());

    /* Direction of the coordinate system is going 'north' (-90°) */
    neutralAngle = -90;

    /* Positive angles go clockwise by default, i.e. this needs to be false */
    counterClockwise = false;
}
