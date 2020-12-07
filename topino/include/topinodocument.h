#ifndef TOPINODOCUMENT_H
#define TOPINODOCUMENT_H

#include "include/iobserver.h"
#include "include/topinodata.h"

#include <QDateTime>
#include <QImage>
#include <QString>
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
        ParsingError = 5
    };

    FileError loadFromXML(const QString &xmlfilename );
    FileError saveToXML( const QString &xmlfilename = "");

    QString getFilename() const;
    void setFilename(const QString& value);

    QString getPath() const;
    void setPath(const QString& value);

    void setFullFilename(const QString& value);
    bool hasFileName() const;

    const TopinoData& getData() const;
    void getData(TopinoData &value) const;
    void setData(const TopinoData& value);

    /* Creates the textual representation of the data header (over view of
     * raw data and fitting parameters) */
    void createDataHeader(QStringList &textData) const;

    /* Creates the textual representation of the raw data + fitted plots */
    void createDataTable(QStringList &textData) const;

    /* Export data to a text file */
    void exportDataToText(const QString &filename) const;

  private:
    std::vector<IObserver*> observers;

    /* This is the version string saved in the document file; currently not checked */
    QString version = "1.0";

    /* True if file has been changed since the last save/open/create */
    bool changed = false;

    /* File name and path of the current document */
    QString filename;
    QString path;

    /* Data object that includes the data methods */
    TopinoData data;

    /* Reads the <topino> object from an XML file */
    FileError readTopinoXML(QXmlStreamReader &xml);

    /* Saves all information as <topino> object to a XML */
    FileError saveTopinoXML(QXmlStreamWriter &xml);
};

#endif // TOPINODOCUMENT_H
