#include "include/topinodata.h"

TopinoData::TopinoData() {
    nextInletID = 1;

    /* Set default values for processing */
    resetProcessing();

    mainInletID = 0;
    neutralAngle = -90;
    minAngle = -30;
    maxAngle = +30;
    counterClockwise = false;
}

TopinoData::~TopinoData() {

}

QImage TopinoData::getImage() const {
    return sourceImage;
}

void TopinoData::setImage(const QImage& value) {
    sourceImage = value;
    processedImage = value;
}

QPointF TopinoData::getCoordOrigin() const {
    return getInletData(mainInletID).coord;
}

void TopinoData::setCoordOrigin(const QPointF& value) {
    TopinoData::InletData data = getInletData(mainInletID);
    data.coord = value;
    updateInlet(data);
}

int TopinoData::getMainInletID() const {
    return mainInletID;
}

void TopinoData::setMainInletID(int value) {
    mainInletID = value;
}

int TopinoData::getCoordNeutralAngle() const {
    return neutralAngle;
}

void TopinoData::setCoordNeutralAngle(int value) {
    neutralAngle = value;
}

int TopinoData::getCoordMinAngle() const {
    return minAngle;
}

void TopinoData::setCoordMinAngle(int value) {
    minAngle = value;
}

int TopinoData::getCoordMaxAngle() const {
    return maxAngle;
}

void TopinoData::setCoordMaxAngle(int value) {
    maxAngle = value;
}

bool TopinoData::getCoordCounterClockwise() const {
    return counterClockwise;
}

void TopinoData::setCoordCounterClockwise(bool value) {
    counterClockwise = value;
}

QList<TopinoData::InletData> TopinoData::getInlets() const {
    return inlets;
}

void TopinoData::setInlets(const QList<TopinoData::InletData>& value) {
    inlets = value;
}

int TopinoData::updateInlet(const TopinoData::InletData& data, bool create) {
    /* Look through all inlets, check if the data can be overwritten */
    for (auto iter = inlets.begin(); iter != inlets.end(); ++iter) {
        /* If the data ID is found, simply overwrite the data with the new data */
        if ((*iter).ID == data.ID) {
            qDebug("Data: updating inlet %d", data.ID);
            qDebug("      new coordinates: %.0f x %.0f", data.coord.x(), data.coord.y());
            qDebug("      before the update: %.0f x %.0f", inlets[iter - inlets.begin()].coord.x(),
                   inlets[iter - inlets.begin()].coord.y());
            inlets[iter - inlets.begin()] = data;
            qDebug("      after the update: %.0f x %.0f", inlets[iter - inlets.begin()].coord.x(),
                   inlets[iter - inlets.begin()].coord.y());
            return data.ID;
        }
    }

    /* Was not found, but caller wants to create anyway; in this case, we
     * ignore the ID of the data given and use our own */
    if (create) {
        qDebug("Inlets: %d", inlets.size());
        TopinoData::InletData newData = data;
        newData.ID = nextInletID;
        inlets.append(newData);
        nextInletID++;
        return newData.ID;
    }

    /* Did not find anything to update and did not add any data! */
    return 0;
}

TopinoData::InletData TopinoData::getInletData(int ID) const {
    /* Look through all inlets and get the data */
    for (auto iter = inlets.begin(); iter != inlets.end(); ++iter) {
        /* If the data ID is found, simply return the data */
        if ((*iter).ID == ID) {
            return inlets[iter - inlets.begin()];
        }
    }

    /* Return empty inlet data object if not found */
    return TopinoData::InletData();
}

TopinoData::ParsingError TopinoData::loadObject(QXmlStreamReader& xml) {
    /* Central function that loads the respective object depending on the name of the current
     * XML element; this function gets usually called by the document object */
    if (xml.name() == "sourceImage") {
        return loadImageObject(xml);
    } else if (xml.name() == "coordinateSystem") {
        return loadCoordinateObject(xml);
    } else if (xml.name() == "inlets") {
        return loadInletsObject(xml);
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
            sourceImage = QImage::fromData(bytes, "PNG");
            processedImage = sourceImage;

            if (sourceImage.isNull())
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
    sourceImage.save(&buffer, "PNG");
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

        if (xml.name() == "neutralAngle") {
            neutralAngle = content.toInt();
        } else if (xml.name() == "minAngle") {
            minAngle = content.toInt();
        } else if (xml.name() == "maxAngle") {
            maxAngle = content.toInt();
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

    xml.writeTextElement("neutralAngle", QString::number(neutralAngle));
    xml.writeTextElement("minAngle", QString::number(minAngle));
    xml.writeTextElement("maxAngle", QString::number(maxAngle));
    xml.writeTextElement("counterClockwise", counterClockwise ? "true" : "false");

    xml.writeEndElement();
}

TopinoData::ParsingError TopinoData::loadInletsObject(QXmlStreamReader& xml) {
    /* Read all inlets from an inlets object */
    while (xml.readNextStartElement()) {
        qDebug("Found inlet %s in inlets object.", xml.name().toString().toStdString().c_str());

        if(xml.name() == "inlet") {
            loadInletObject(xml);
        } else {
            xml.skipCurrentElement();
        }
    }

    /* No parsing error while loading the image */
    return ParsingError::NoFailure;
}

void TopinoData::saveInletsObject(QXmlStreamWriter& xml) {
    qDebug("Saving %d inlets...", inlets.size());
    /* Save each inlet */
    xml.writeStartElement("inlets");

    for(auto iter = inlets.begin(); iter != inlets.end(); ++iter) {
        saveInletObject(xml, *iter);
    }

    xml.writeEndElement();
}

TopinoData::ParsingError TopinoData::loadInletObject(QXmlStreamReader& xml) {
    /* Prepare data */
    TopinoData::InletData data;

    /* Check if the attribute for main inlet is here; has to be checked here, otherwise
     * it will check the subitems for the attribute instead */
    bool isMainInlet = xml.attributes().hasAttribute("mainInlet");

    /* Read all elements of the inlet object and fill in the respective members; only
     * non main inlets are created this way! */
    while (xml.readNextStartElement()) {
        /* Read content of element and parse it */
        QString content = xml.readElementText().toLower();

        qDebug("Found element %s in coordinate object: %s.",
               xml.name().toString().toStdString().c_str(),
               content.toStdString().c_str());

        if(xml.name() == "coordX") {
            data.coord.setX(content.toDouble());
        } else if (xml.name() == "coordY") {
            data.coord.setY(content.toDouble());
        } else if (xml.name() == "radius") {
            data.radius = content.toInt();
        } else {
            xml.skipCurrentElement();
        }
    }

    /* Only add if radius > 0 */
    if (data.radius > 0) {
        data.ID = nextInletID;
        inlets.append(data);
        nextInletID++;

        /* Has attribute to be the main inlet? then save this; there can only be one main
         * inlet and if the file has more than one defined, the last one read will be the
         * main inlet */
        if (isMainInlet) {
            mainInletID = data.ID;
        }
    }

    /* No parsing error while loading the image */
    return ParsingError::NoFailure;
}

void TopinoData::saveInletObject(QXmlStreamWriter& xml, const InletData &data) {
    qDebug("saving inlet with ID %d", data.ID);

    xml.writeStartElement("inlet");

    /* This inlet is the main inlet, so mark it! */
    if (data.ID == mainInletID) {
        xml.writeAttribute("mainInlet", "true");
    }

    xml.writeTextElement("coordX", QString::number(data.coord.x()));
    xml.writeTextElement("coordY", QString::number(data.coord.y()));
    xml.writeTextElement("radius", QString::number(data.radius));

    xml.writeEndElement();
}

bool TopinoData::getInversion() const {
    return inversion;
}

void TopinoData::setInversion(bool value) {
    inversion = value;
}

TopinoTools::desaturationModes TopinoData::getDesatMode() const {
    return desatMode;
}

void TopinoData::setDesatMode(const TopinoTools::desaturationModes& value) {
    desatMode = value;
}

int TopinoData::getLevelMin() const {
    return levelMin;
}

void TopinoData::setLevelMin(int value) {
    levelMin = value;
}

int TopinoData::getLevelMax() const {
    return levelMax;
}

void TopinoData::setLevelMax(int value) {
    levelMax = value;
}

QImage TopinoData::getProcessedImage() const {
    return processedImage;
}

void TopinoData::setProcessedImage(const QImage& value) {
    processedImage = value;
}

void TopinoData::processImage() {
    /* Start with the source image */
    processedImage = sourceImage;

    /* Invert if needed */
    if (inversion) {
        processedImage.invertPixels();
    }

    /* Iterate over all pixels and apply desaturation method. Count the
     * values for the histogram in the same run. */
    int pixelCount = processedImage.width() * processedImage.height();
    QRgb *pixels = reinterpret_cast<QRgb *>(processedImage.bits());

    /* Process all pixels individually */
    qreal scale = 255.0 / (qreal)(levelMax - levelMin);
    for (int p = 0; p < pixelCount; ++p) {
        /* The value calculated depends on the method selected */
        int value = 0;

        switch(desatMode) {
        /* Luminance method */
        case TopinoTools::desaturationModes::desatLuminance:
            value = TopinoTools::qLuminance(pixels[p]);
            break;

        /* Average method */
        case TopinoTools::desaturationModes::desatAverage:
            value = TopinoTools::qAverage(pixels[p]);
            break;

        /* Maximum method */
        case TopinoTools::desaturationModes::desatMaximum:
            value = TopinoTools::qMaximum(pixels[p]);
            break;

        /* Red channel */
        case TopinoTools::desaturationModes::desatRed:
            value = qRed(pixels[p]);
            break;

        /* Green channel */
        case TopinoTools::desaturationModes::desatGreen:
            value = qGreen(pixels[p]);
            break;

        /* Blue channel */
        case TopinoTools::desaturationModes::desatBlue:
            value = qBlue(pixels[p]);
            break;

        /* Lightness method (default) */
        case TopinoTools::desaturationModes::desatLightness:
        default:
            value = TopinoTools::qLightness(pixels[p]);
            break;
        }

        /* Subtract the bottom value; since all the channels are the same
         * it does not really matter which channel we use here. */
        value = qMax(0, value - levelMin);

        /* Multiply with the scale */
        value = qMin(255, (int)(value * scale));

        /* Apply new desaturated value */
        pixels[p] = qRgb(value, value, value);
    }
}

void TopinoData::resetProcessing() {
    /* Default values for processing the image */
    inversion = false;
    desatMode = TopinoTools::desaturationModes::desatLightness;
    levelMin = 0;
    levelMax = 255;

    /* Reset image as well */
    processedImage = sourceImage;
}



