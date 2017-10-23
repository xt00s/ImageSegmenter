#include "Scheme.h"
#include <QJsonArray>

Scheme::~Scheme()
{
	qDeleteAll(categories_);
}

Category *Scheme::findCategory(const QString &name) const
{
	auto lower = name.toLower();
	for (auto& c : categories_)
		if (c->name().toLower() == lower)
			return c;
	return 0;
}

Scheme* Scheme::fromJson(const QJsonObject &json)
{
	auto s = new Scheme;
	s->name_ = json["Name"].toString();
	for (auto& r : json["Categories"].toArray())
		s->categories_ << Category::fromJson(r.toObject());
	s->updateIndexes();
	return s;
}

QJsonObject Scheme::toJson() const
{
	QJsonObject json;
	json["Name"] = name_;
	QJsonArray regions;
	for (auto& c : categories_)
		regions << c->toJson();
	json["Categories"] = regions;
	return json;
}

void Scheme::updateIndexes()
{
	int i = 0;
	for (auto& c : categories_) {
		c->index_ = i++;
	}
}

void Scheme::addCategory(Category *category)
{
	categories_ << category;
	updateIndexes();
}

void Scheme::removeCategory(Category *category)
{
	categories_.removeOne(category);
	delete category;
	updateIndexes();
}
