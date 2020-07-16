#include "include/topinodata.h"

TopinoData::TopinoData() {

}

TopinoData::~TopinoData() {

}

QImage TopinoData::getImage() const {
    return image;
}

void TopinoData::setImage(const QImage& value) {
    image = value;
}

TopinoData::ParsingError TopinoData::loadObject(QXmlStreamReader& xml) {
    /* Central function that loads the respective object depending on the name of the current
     * XML element */
    if (xml.name() == "sourceImage") {
            return loadImageObject(xml);
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
