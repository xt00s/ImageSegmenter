#ifndef SCHEME_H
#define SCHEME_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include "Category.h"

class Scheme
{
public:
    ~Scheme();

    QString name() const;
    void setName(const QString& name);

    QList<Category*> categories() const;
    int categoryCount() const;
    Category* categoryAt(int i) const;
    void addCategory(Category* category);
    void removeCategory(Category* category);
    Category* findCategory(const QString& name) const;

    static Scheme* fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

private:
    void updateIndexes();

private:
    QString name_;
    QList<Category*> categories_;
};

// inlines

inline QString Scheme::name() const { return name_; }
inline void Scheme::setName(const QString &name) { name_ = name; }
inline QList<Category*> Scheme::categories() const { return categories_; }
inline int Scheme::categoryCount() const { return categories_.count(); }
inline Category*Scheme::categoryAt(int i) const { return categories_[i]; }

#endif // SCHEME_H
