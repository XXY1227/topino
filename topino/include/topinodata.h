#ifndef TOPINODATA_H
#define TOPINODATA_H

#include <QBuffer>
#include <QImage>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class TopinoData {
  public:
    TopinoData();
    ~TopinoData();

    enum class ParsingError {
        NoFailure = 0,
        IgnoredElement = 1,
        CouldNotLoadImage = 2
    };

    QImage getImage() const;
    void setImage(const QImage& value);

    ParsingError loadObject(QXmlStreamReader& xml);

    ParsingError loadImageObject(QXmlStreamReader& xml);
    void saveImageObject(QXmlStreamWriter& xml);

    ParsingError loadCoordinateObject(QXmlStreamReader& xml);
    void saveCoordinateObject(QXmlStreamWriter& xml);

  private:
    /* Original and unmodified image */
    QImage image;

    /* Polar coordinate system: origin coordinates on image, neutral plane angle (given in degrees),
     * and if direction of increasing angles is counterClockwise (true/false) */
    QPointF origin;
    int neutralAngle;
    bool counterClockwise;

    void createCoordinateSystem();
};

#endif // TOPINODATA_H
