#include "include/topinodata.h"

TopinoData::TopinoData() {
    nextInletID = 1;

    /* Set default values for processing */
    resetProcessing();

    mainInletID = 0;
    neutralAngle = -90;
    minAngle = -30;
    maxAngle = +30;
    diffAngle = 15;
    outerRadius = 120;
    counterClockwise = false;
    sectors = 3;
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

int TopinoData::addInlet(const TopinoData::InletData& data) {
    /* Function for convenience */
    return updateInlet(data, true);
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

void TopinoData::removeInlet(int ID) {
    /* Removes the inlet data with ID if it exists. */
    for (int index = 0; index < inlets.size(); ++index) {
        /* If the data ID is found, remove it, and break out of loop
         * since the indices are now invalid anyway. */
        if (inlets[index].ID == ID) {
            inlets.removeAt(index);

            /* If this inlet was the maininlet, we need to reset
             * some data here, too. */
            if (mainInletID == ID) {
                mainInletID = 0;
                angulagramPoints.clear();
                streamParameters.clear();
            };

            break;
        }
    }
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
    } else if (xml.name() == "stream") {
        return loadStreamParameters(xml);
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

            continue;
        }

        /* If not image data -> read content of element and parse it */
        QString content = xml.readElementText().toLower();

        if (xml.name() == "desatmode") {
            int value = content.toInt();
            if (value < TopinoTools::desaturationModes::desatCOUNT)
                desatMode = static_cast<TopinoTools::desaturationModes>(value);
        } else if (xml.name() == "inversion") {
            inversion = content.toInt() > 0;
        } else if (xml.name() == "levelMin") {
            levelMin = content.toInt();
        } else if (xml.name() == "levelMax") {
            levelMax = content.toInt();
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

    /* Save the processing data, i.e. mode, min and max level; here, we also add a description
     * for the mode enum, so that the XML can be read by humans */
    xml.writeStartElement ("desatmode");
        xml.writeAttribute ("desc", TopinoTools::getDesaturationModeName(desatMode));
        xml.writeCharacters(QString::number(desatMode));
    xml.writeEndElement ();
    xml.writeTextElement("inversion", QString::number(inversion));
    xml.writeTextElement("levelMin", QString::number(levelMin));
    xml.writeTextElement("levelMax", QString::number(levelMax));

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
        } else if (xml.name() == "diffAngle") {
            diffAngle = content.toInt();
        } else if (xml.name() == "outerRadius") {
            outerRadius = content.toInt();
        } else if (xml.name() == "counterClockwise") {
            counterClockwise = (content.compare("true") == 0);
        } else if (xml.name() == "sectors") {
            sectors = content.toInt();
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
    xml.writeTextElement("diffAngle", QString::number(diffAngle));
    xml.writeTextElement("outerRadius", QString::number(outerRadius));
    xml.writeTextElement("counterClockwise", counterClockwise ? "true" : "false");
    xml.writeTextElement("sectors", QString::number(sectors));

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

TopinoData::ParsingError TopinoData::loadStreamParameters(QXmlStreamReader& xml) {
    /* Read all elements of the stream object and fill in the respective members */
    TopinoTools::Lorentzian data;

    while (xml.readNextStartElement()) {
        /* Read content of element and parse it */
        QString content = xml.readElementText().toLower();

        qDebug("Found element %s in stream object: %s.",
               xml.name().toString().toStdString().c_str(),
               content.toStdString().c_str());

        if (xml.name() == "pos") {
            data.pos = content.toDouble();
        } else if (xml.name() == "width") {
            data.width = content.toDouble();
        } else if (xml.name() == "height") {
            data.height = content.toDouble();
        } else if (xml.name() == "offset") {
            data.offset = content.toDouble();
        } else if (xml.name() == "rsquare") {
            data.rsquare = content.toDouble();
        } else {
            xml.skipCurrentElement();
        }
    }

    /* Add to data */
    streamParameters.append(data);

    /* No parsing error while loading the image */
    return ParsingError::NoFailure;
}

void TopinoData::saveStreamParameters(QXmlStreamWriter& xml) {
    /* Save the stream parameters */
    for(auto it = streamParameters.begin(); it != streamParameters.end(); ++it) {
        xml.writeStartElement("stream");

        xml.writeTextElement("pos", QString::number(it->pos));
        xml.writeTextElement("width", QString::number(it->width));
        xml.writeTextElement("height", QString::number(it->height));
        xml.writeTextElement("offset", QString::number(it->offset));
        xml.writeTextElement("rsquare", QString::number(it->rsquare));

        xml.writeEndElement();
    }
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

void TopinoData::calculatePolarImage() {
    qDebug("Calculate polar image");

    /* Check for the main inlet. If not defined, we set the polar image to
     * a null image so that follow-up functions do not process garbage data. */
    if (mainInletID == 0) {
        qDebug("No main inlet defined. Did not calculate a polar image.");
        polarImage = QImage();

        return;
    }

    /* Need the data of the main inlet (radii, etc) */
    TopinoData::InletData mainInletData = getInletData(mainInletID);

    /* Create a new image that has the angles as heights (0.1° steps, so multiply
     * by 10) and the radii as widths. Is will be RGB32 color - we want to be able
     * to display the image to the user if needed. Fill everything with zeros (black). */
    int angleSteps = (maxAngle - minAngle) * 10;
    polarImage = QImage(outerRadius, angleSteps, QImage::Format_RGB32);
    polarImage.fill(Qt::black);

    /* Calculate the signal for each radian and angle and set the respective pixel on the
     * image to a RGB color (each channel intensity = signal). Using the direct access to
     * bit data of the image ensures high performance. */
    QRgb *polarPixels = reinterpret_cast<QRgb *>(polarImage.bits());
    QRgb *processedPixels = reinterpret_cast<QRgb *>(processedImage.bits());
    for (int r = 0; r < outerRadius; ++r) {
        for (int a = 0; a < angleSteps; ++a ) {
            /* Ignore the inner radius of the inlet (garbage data) */
            if (r < mainInletData.radius)
                continue;

            /* Current "real" angle */
            qreal angle = neutralAngle + minAngle + a * 0.1;

            /* Calculate x and y of these polar coordinates; translate the point by the
             * origin. Keep in mind that the y coordinates start at the top to bottom,
             * but the mathematical point is from bottom to top (therefore, the minus)! */
            int x = mainInletData.coord.x() + int(qRound(r * qCos(qDegreesToRadians(angle))));
            int y = mainInletData.coord.y() - int(qRound(r * qSin(qDegreesToRadians(angle))));

            /* Get the intensity. Since the processed image should have the same signal
             * in all channels, it is ok just to use the green here. Also, we should make
             * sure that the x and y coordinates are inside the image before reading out! */
            int intensity = 0;

            if ((x > 0) && (x < processedImage.width()) && (y > 0) && (y < processedImage.height())) {
                intensity = qGreen(processedPixels[y * processedImage.width() + x]);
            }

            /* Write to the polar pixels. Here, x = r and y = a */
            polarPixels[a * polarImage.width() + r] = qRgb(intensity, intensity, intensity);
        }
    }
}

void TopinoData::calculateAngulagramPoints() {
    qDebug("Calculate angulagram points");

    /* Clear old points */
    angulagramPoints.clear();

    /* Make sure the image has been created */
    if (polarImage.isNull() || (mainInletID == 0)) {
        qDebug("No polar image or main inlet defined. No angulagram points calculated.");

        return;
    }

    /* Process all the data of the image and integrate over the x-axis (radius) */
    QRgb *polarPixels = reinterpret_cast<QRgb *>(polarImage.bits());
    int width = polarImage.width();
    int height = polarImage.height();

    /* Factor to multiply into the points. Min angle could be plus or negative depending
     * on counterclockwise */
    qreal xFactor = counterClockwise ? 1.0 : -1.0;

    for (int y = 0; y < height; ++y) {
        /* Integrate over x-axis (radius). Again, the intensities of each channel
         * should be the same, so we just take the green channel here. */
        int intensity = 0;

        for (int x = 0; x < width; ++x) {
            intensity += qGreen(polarPixels[y * width + x]);
        }

        /* Finally, add the data point to the angulagram data. The angle is
         * minAngle + y * 0.1° - this is how we created the image in the
         * previous step (see calculatePolarImage() above). */
        angulagramPoints.append(QPointF((minAngle + y * 0.1) * xFactor, intensity));
    }

    qDebug("Angulagram has been calculated");
}

QVector<QPointF> TopinoData::getAngulagramPoints() const {
    return angulagramPoints;
}

void TopinoData::calculateRadialgramPoints() {
    qDebug("Calculate radialgram points");

    /* Clear old points */
    radialgramPoints.clear();

    /* Make sure the image has been created */
    if (polarImage.isNull() || (mainInletID == 0)) {
        qDebug("No polar image or main inlet defined. No radialgram points calculated.");

        return;
    }

    /* Process all the data of the image and integrate over the x-axis (radius) */
    QRgb *polarPixels = reinterpret_cast<QRgb *>(polarImage.bits());
    int width = polarImage.width();
    int height = polarImage.height();

    /* Simply go over the image x-axis (= radius) and integrate over y. In this case
     * the angle sign etc. does not matter. */
    for (int x = 0; x < width; ++x) {
        /* Integrate over y-axis (angle). Again, the intensities of each channel
         * should be the same, so we just take the green channel here. */
        int intensity = 0;

        for (int y = 0; y < height; ++y) {
            intensity += qGreen(polarPixels[y * width + x]);
        }

        /* Finally, add the data point to the radialgram data. */
        radialgramPoints.append(QPointF(x * 1.0, intensity));
    }

    qDebug("Radialgram has been calculated with %d points.", radialgramPoints.length());
}

QVector<QPointF> TopinoData::getRadialgramPoints() const {
    return radialgramPoints;
}

int TopinoData::getCoordOuterRadius() const {
    return outerRadius;
}

void TopinoData::setCoordOuterRadius(int value) {
    outerRadius = value;
}

bool TopinoData::isAngulagramAvailable() const {
    return angulagramPoints.length() > 0;
}

QVector<TopinoTools::Lorentzian> TopinoData::getStreamParameters() const {
    return streamParameters;
}

void TopinoData::setStreamParameters(const QVector<TopinoTools::Lorentzian>& value) {
    streamParameters = value;
}

QImage TopinoData::getPolarImage() const {
    return polarImage;
}

int TopinoData::getCoordDiffAngle() const {
    return diffAngle;
}

void TopinoData::setCoordDiffAngle(int value) {
    diffAngle = value;
}

int TopinoData::getCoordSectors() const {
    return sectors;
}

void TopinoData::setCoordSectors(int value) {
    sectors = value;
}



