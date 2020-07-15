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

  private:
    QImage image;
};

#endif // TOPINODATA_H
