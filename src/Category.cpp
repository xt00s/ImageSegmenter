#include "Category.h"
#include "Helper.h"

Category::Category()
	: index_(-1), visible_(true), locked_(false)
{}

Category* Category::fromJson(const QJsonObject &json)
{
	auto c = new Category;
	c->name_ = json["Name"].toString();
	c->color_ = QColor(json["Color"].toString());
	c->color_.setAlpha(255);
	return c;
}

QJsonObject Category::toJson() const
{
	QJsonObject json;
	json["Name"] = name_;
	json["Color"] = color_.name();
	return json;
}
