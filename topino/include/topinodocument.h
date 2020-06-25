#ifndef TOPINODOCUMENT_H
#define TOPINODOCUMENT_H

#include <include/iobserver.h>

#include <QString>
#include <QImage>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <vector>

class TopinoDocument {
  public:
    TopinoDocument();
    ~TopinoDocument();

    void addObserver(IObserver *observer);
    void removeObserver(IObserver *observer);
    void notifyAllObserver() const;

    bool hasChanged() const;
    void modify();
    void saveChanges();

    enum class FileError {
        NoFailure = 0,
        UnknownError = 1,
        FileNotFound = 2,
        FileNameOrPathNotSet = 3,
        CouldNotOpen = 4,
        CouldNotLoadImage = 5,
        ParsingError = 6
    };

    FileError loadFromXML(const QString &xmlfilename );
    FileError saveToXML( const QString &xmlfilename = "");

    QString getFilename() const;
    void setFilename(const QString& value);

    QString getPath() const;
    void setPath(const QString& value);

    void setFullFilename(const QString& value);
    bool hasFileName() const;

    QImage getImage() const;
    void setImage(const QImage& value);

  private:
    std::vector<IObserver*> observers;

    /* True if file has been changed since the last save/open/create */
    bool changed = false;

    /* File name and path of the current document */
    QString filename;
    QString path;

    /* TODO: Move to data object */
    QImage image;

    /* Reads the <topino> object from an XML file */
    FileError readTopinoXML(QXmlStreamReader &xml);

    /* Saves all information as <topino> object to a XML */
    FileError saveTopinoXML(QXmlStreamWriter &xml);
};

#endif // TOPINODOCUMENT_H
