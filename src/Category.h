#ifndef CATEGORY_H
#define CATEGORY_H

#include <QString>
#include <QColor>
#include <QJsonObject>

class Category
{
public:
    Category();

    QString name() const;
    void setName(const QString& name);

    QColor color() const;
    void setColor(QColor color);

    bool isVisible() const;
    void setVisible(bool visible);

    bool isLocked() const;
    void setLocked(bool locked);

    int index() const;

    static Category* fromJson(const QJsonObject& json);
    QJsonObject toJson() const;

private:
    friend class Scheme;
    Q_DISABLE_COPY(Category)
    int index_;
    QString name_;
    QColor color_;
    bool visible_;
    bool locked_;
};

// inlines

inline QString Category::name() const { return name_; }
inline void Category::setName(const QString &name) { name_ = name; }
inline QColor Category::color() const { return color_; }
inline void Category::setColor(QColor color) { color_ = color; }
inline bool Category::isVisible() const { return visible_; }
inline void Category::setVisible(bool visible) { visible_ = visible; }
inline bool Category::isLocked() const { return locked_; }
inline void Category::setLocked(bool locked) { locked_ = locked; }
inline int Category::index() const { return index_; }

#endif // CATEGORY_H
