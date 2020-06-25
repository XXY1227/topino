#include "include/topinodocument.h"

#include <algorithm>

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QBuffer>

TopinoDocument::TopinoDocument() {
    filename = QObject::tr("Unnamed");
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

void TopinoDocument::notifyAllObserver() const {
    for (auto observer : observers)
        observer->modelHasChanged();
}

bool TopinoDocument::hasChanged() const {
    return changed;
}

void TopinoDocument::modify() {
    changed = true;
    notifyAllObserver();
}

void TopinoDocument::saveChanges() {
    changed = false;
}

TopinoDocument::FileError TopinoDocument::loadFromXML(const QString& xmlfilename) {
    /* Try to open the file from xmlfilename, which should
       include the whole path */
    QFile f(xmlfilename);

    if (!f.exists()) {
        return FileError::FileNotFound;
    }

    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        return FileError::CouldNotOpen;
    }

    /* Main loop for reading the XML file */
    QXmlStreamReader xml(&f);

    while (!xml.atEnd()) {
        if (xml.readNext() != QXmlStreamReader::EndDocument) {
            if (xml.isStartElement() && xml.name() == "topino") {
                FileError err = readTopinoXML(xml);

                if (err != FileError::NoFailure)
                    return err;
            }
        }
    }

    /* Check for errors */
    if (xml.hasError()) {
        /* TODO: do more error handling */
        qDebug("XML errors = %d: %s", xml.error(), xml.errorString().toStdString().c_str());
        return FileError::ParsingError;
    }
    f.close();

    /* Split the full path into filename and path */
    QFileInfo fi(xmlfilename);
    filename = fi.fileName();
    path = fi.absolutePath();

    /* Freshly opened files are not changed (yet) */
    changed = false;

    return FileError::NoFailure;
}

TopinoDocument::FileError TopinoDocument::saveToXML(const QString& xmlfilename) {
    /* Use the full filename given to the function if available; split into components */
    if (xmlfilename.length() > 0) {
        QFileInfo fi(xmlfilename);
        filename = fi.fileName();
        path = fi.absolutePath();
    }

    /* File name AND path must be set to save the file somewhere */
    if (filename.length() == 0 || path.length() == 0)
        return FileError::FileNameOrPathNotSet;

    QFile f(path + "/" + filename);

    if (!f.open(QFile::WriteOnly))
        return FileError::CouldNotOpen;

    /* Create basic XML structure and add <topino> object to it */
    QXmlStreamWriter xml(&f);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    saveTopinoXML(xml);

    /* Finish document and close file */
    xml.writeEndDocument();
    f.close();

    /* Was saved; remove the modification flag */
    changed = false;

    return FileError::NoFailure;
}

QString TopinoDocument::getFilename() const {
    return filename;
}

void TopinoDocument::setFilename(const QString& value) {
    filename = value;
    modify();
}

QString TopinoDocument::getPath() const {
    return path;
}

void TopinoDocument::setPath(const QString& value) {
    path = value;
    modify();
}

void TopinoDocument::setFullFilename(const QString& value) {
    QFileInfo fi(value);
    filename = fi.fileName();
    path = fi.absolutePath();
    modify();
}

bool TopinoDocument::hasFileName() const {
    return (filename.length() > 0 && path.length() > 0);
}

QImage TopinoDocument::getImage() const {
    return image;
}

void TopinoDocument::setImage(const QImage& value) {
    image = value;
    modify();
}

TopinoDocument::FileError TopinoDocument::readTopinoXML(QXmlStreamReader& xml) {
    /* Read all elements */
    while (xml.readNextStartElement()) {
        qDebug("Found element %s in XML...", xml.name().toString().toStdString().c_str());

        /* The source image is saved as base64 encoded PNG */
        if(xml.name() == "sourceImage") {
            QString text = xml.readElementText();
            QByteArray bytes;
            bytes.append(text);
            bytes = QByteArray::fromBase64(bytes);
            image = QImage::fromData(bytes, "PNG");

            if (image.isNull())
                return FileError::CouldNotLoadImage;
        } else {
            xml.skipCurrentElement();
        }
    }

    return FileError::NoFailure;
}

TopinoDocument::FileError TopinoDocument::saveTopinoXML(QXmlStreamWriter& xml) {
    xml.writeStartElement("topino");

    /* Convert the image (through a QBuffer) to a base64 encoded PNG and write it down as text element */
    QByteArray bytes;
    QBuffer buffer(&bytes);
    image.save(&buffer, "PNG");
    xml.writeTextElement("sourceImage", bytes.toBase64());

    xml.writeEndElement();

    return FileError::NoFailure;
}
