#ifndef TOPINODATA_H
#define TOPINODATA_H

#include <QBuffer>
#include <QImage>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "include/topinotool.h"

class TopinoData {
  public:
    TopinoData();
    ~TopinoData();

    enum class ParsingError {
        NoFailure = 0,
        IgnoredElement = 1,
        CouldNotLoadImage = 2
    };

    struct InletData {
        int ID;
        QPointF coord;
        int radius;
    };

    /* Setter and getter for data objects */
    QImage getImage() const;
    void setImage(const QImage& value);

    QPointF getCoordOrigin() const;
    void setCoordOrigin(const QPointF& value);

    int getMainInletID() const;
    void setMainInletID(int value);

    int getCoordNeutralAngle() const;
    void setCoordNeutralAngle(int value);

    int getCoordMinAngle() const;
    void setCoordMinAngle(int value);

    int getCoordMaxAngle() const;
    void setCoordMaxAngle(int value);

    bool getCoordCounterClockwise() const;
    void setCoordCounterClockwise(bool value);

    QList<InletData> getInlets() const;
    void setInlets(const QList<InletData>& value);
    int updateInlet(const InletData &data, bool create = false);
    TopinoData::InletData getInletData(int ID) const;

    /* XML functions for loading and saving */
    ParsingError loadObject(QXmlStreamReader& xml);

    ParsingError loadImageObject(QXmlStreamReader& xml);
    void saveImageObject(QXmlStreamWriter& xml);

    ParsingError loadCoordinateObject(QXmlStreamReader& xml);
    void saveCoordinateObject(QXmlStreamWriter& xml);

    ParsingError loadInletsObject(QXmlStreamReader& xml);
    void saveInletsObject(QXmlStreamWriter& xml);

    ParsingError loadInletObject(QXmlStreamReader& xml);
    void saveInletObject(QXmlStreamWriter& xml, const InletData &data);

    bool getInversion() const;
    void setInversion(bool value);

    TopinoTools::desaturationModes getDesatMode() const;
    void setDesatMode(const TopinoTools::desaturationModes& value);

    int getLevelMin() const;
    void setLevelMin(int value);

    int getLevelMax() const;
    void setLevelMax(int value);

    QImage getProcessedImage() const;
    void setProcessedImage(const QImage& value);

    /* Process the source image and apply inversion, desaturation, and color levels. */
    void processImage();

    /* Resets the processing of the image and sets all values to default */
    void resetProcessing();

  private:
    /* Image and image editing data */
    QImage sourceImage;
    QImage processedImage;

    bool inversion;
    TopinoTools::desaturationModes desatMode;
    int levelMin;
    int levelMax;

    /* Polar coordinate system: origin coordinates on image, neutral plane angle (given in degrees),
     * and if direction of increasing angles is counterClockwise (true/false) */
    int mainInletID;
    int neutralAngle;
    int minAngle;
    int maxAngle;
    bool counterClockwise;

    /* List of inlets */
    QList<InletData> inlets;
    int nextInletID;
};

#endif // TOPINODATA_H
