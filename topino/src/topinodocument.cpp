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

const TopinoData &TopinoDocument::getData() const {
    /* For fast access a const reference is given */
    return data;
}

void TopinoDocument::getData(TopinoData& value) const {
    /* Copy data to the given reference */
    value = data;
}

void TopinoDocument::setData(const TopinoData& value) {
    data = value;
    modify();
}

void TopinoDocument::createDataHeader(QStringList& textData) const {
    /* First, let's put in the name of the file we are evaluating here and then
     * some information about the image */
    textData.append(QObject::tr("Timestamp:") +"\t" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));

    if (hasFileName()) {
        textData.append(QObject::tr("File:") + "\t" + filename);
    }
    textData.append(QString(QObject::tr("Image:") + "\t%1 × %2 Px²").arg(data.getImage().width()).arg(data.getImage().height()));

    TopinoData::InletData inletData = data.getInletData(data.getMainInletID());
    textData.append(QString(QObject::tr("Inlet:") + "\tx:\t%1 Px\ty:\t%2 Px\tRef. angle:\t%3°\tInner radius:\t%4 Px\tOuter radius:\t%5 Px")
                    .arg(qRound(inletData.coord.x())).arg(qRound(inletData.coord.y()))
                    .arg(data.getCoordNeutralAngle()).arg(inletData.radius).arg(data.getCoordOuterRadius()));
    textData.append("");

    /* Stream data: first the data of each stream */
    textData.append("\t\t𝜑\t𝜔\tL²");
    QVector<TopinoTools::Lorentzian> streams = data.getStreamParameters();
    for(int i = 0; i < streams.length(); ++i) {
        QString line;
        line.sprintf("Stream %d\t%+.1f°\t%.1f°\t%.2f", i+1, streams[i].pos, streams[i].width, streams[i].rsquare);
        textData.append(line);
    }
    textData.append("");

    /* Stream data: add the resolutions between each stream */
    textData.append("1. Stream\t2.Stream\tR₁₂");
    for(int i = 0; i < streams.length(); ++i) {
        for(int j = (i+1); j < streams.length(); ++j) {
            QString line;
            line.sprintf("%d\t%d\t%.2f", i+1, j+1, TopinoTools::calculateResolution(
                             streams[i].pos, streams[i].width,
                             streams[j].pos, streams[j].width));
            textData.append(line);
        }
    }
}

void TopinoDocument::createDataTable(QStringList& textData) const {
    /* For each point in data points, we add the respective raw data point and the
    * stream fits. */
    QVector<TopinoTools::Lorentzian> parameters = data.getStreamParameters();
    int fits = parameters.length();

    /* Header of table */
    textData.append(QObject::tr("All data intensities are in arbitrary units."));
    QString header = "𝜑 (°)\traw data int";
    for (int i = 0; i < fits; ++i) {
        header += "\tStream " + QString::number(i+1);
    }
    textData.append(header);

    /* Contents of table */
    QVector<QPointF> points = data.getAngulagramPoints();
    for(int i = 0; i < points.length(); ++i) {
        QStringList line;

        /* Raw data */
        line.append(QString::number(points[i].x()));
        line.append(QString::number(points[i].y()));

        /* Data for the fits */
        for(int j = 0; j < fits; ++j) {
            line.append(QString::number(parameters[j].f(points[i].x())));
        }

        textData.append(line.join("\t"));
    }

}

void TopinoDocument::exportDataToText(const QString& filename) const {
    qDebug("Exporting data to %s", filename.toStdString().c_str());

    /* Prepare all the data in text form, line by line */
    QStringList textdata;
    createDataHeader(textdata);
    textdata.append("");
    createDataTable(textdata);

    /* Write it to the given file */
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write(textdata.join("\n").toUtf8());
        file.close();
    } else {
        qDebug("Could not open file %s", filename.toStdString().c_str());
    }
}

TopinoDocument::FileError TopinoDocument::readTopinoXML(QXmlStreamReader& xml) {
    /* Read all elements */
    while (xml.readNextStartElement()) {
        qDebug("Found element %s in XML...", xml.name().toString().toStdString().c_str());

        if (data.loadObject(xml) == TopinoData::ParsingError::IgnoredElement) {
            xml.skipCurrentElement();
        }
    }

    return FileError::NoFailure;
}

TopinoDocument::FileError TopinoDocument::saveTopinoXML(QXmlStreamWriter& xml) {
    /* Document header <topino> with <version> element */
    xml.writeStartElement("topino");
    xml.writeTextElement("version", version);

    /* Save all the data objects */
    data.saveImageObject(xml);
    data.saveCoordinateObject(xml);
    data.saveInletsObject(xml);

    xml.writeEndElement();

    return FileError::NoFailure;
}
