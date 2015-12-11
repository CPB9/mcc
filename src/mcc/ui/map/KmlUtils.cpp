/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/ui/map/KmlUtils.h"

#include <QColor>
#include <QDebug>

#include <kml/dom/kml_cast.h>
#include <kml/dom/kml_factory.h>
#include <bmcl/Logging.h>

#include <random>

namespace mcc {
namespace ui {
namespace map {

kmldom::StylePtr KmlUtils::resolveStyle(const kmldom::Document* parent, const kmldom::Feature* feature)
{
    if (!feature->has_styleurl()) {
        return nullptr;
    }
    const std::string& styleUrl = feature->get_styleurl();
    return resolveStyle(parent, styleUrl);
}

kmldom::StylePtr KmlUtils::resolveStyle(const kmldom::Document* parent, const std::string& styleUrl)
{
    std::size_t i = 0;
    while (i < parent->get_styleselector_array_size()) {
        const kmldom::StyleSelectorPtr& styleSelector = parent->get_styleselector_array_at(i);
        i++;
        const std::string& id = styleSelector->get_id();

        if (styleUrl.size() != (id.size() + 1)) {
            continue;
        }
        if (!std::equal(styleUrl.begin() + 1, styleUrl.end(), id.begin())) {
            continue;
        }
        if (styleSelector->IsA(kmldom::Type_Style)) {
            return kmldom::AsStyle(styleSelector);
        } else if (styleSelector->IsA(kmldom::Type_StyleMap)) {
            kmldom::StyleMapPtr styleMap = kmldom::AsStyleMap(styleSelector);
            for (std::size_t j = 0; j < styleMap->get_pair_array_size(); j++) {
                kmldom::PairPtr pair = styleMap->get_pair_array_at(j);
                if (pair->has_key() && pair->has_styleurl()) {
                    int key = pair->get_key();
                    if (key == kmldom::STYLESTATE_NORMAL) {
                        const std::string& nextUrl = pair->get_styleurl();
                        return resolveStyle(parent, nextUrl);
                    }
                }
            }
        }
    }
    return nullptr;
}

kmldom::CoordinatesPtr KmlUtils::resolveCoordinates(const kmldom::Placemark* pm)
{
    kmldom::GeometryPtr geometry = pm->get_geometry();
    if (geometry->IsA(kmldom::Type_Point)) {
        kmldom::PointPtr point = kmldom::AsPoint(geometry);
        if (point->has_coordinates()) {
            return point->get_coordinates();
        }
    } else if (geometry->IsA(kmldom::Type_LineString)) {
        kmldom::LineStringPtr ls = kmldom::AsLineString(geometry);
        if (ls->has_coordinates()) {
            return ls->get_coordinates();
        }
    } else if (geometry->IsA(kmldom::Type_Polygon)) {
        kmldom::PolygonPtr polygon = kmldom::AsPolygon(geometry);
        if (polygon->has_outerboundaryis()) {
            kmldom::OuterBoundaryIsPtr ob = polygon->get_outerboundaryis();
            if (ob->has_linearring()) {
                kmldom::LinearRingPtr lr = ob->get_linearring();
                if (lr->has_coordinates()) {
                    return lr->get_coordinates();
                }
            }
        }
    }
    return nullptr;
}

kmlbase::Color32 KmlUtils::kmlColorFromQcolor(const QColor& qcolor)
{
    kmlbase::Color32 color;
    color.set_alpha(qcolor.alpha());
    color.set_red(qcolor.red());
    color.set_green(qcolor.green());
    color.set_blue(qcolor.blue());
    return color;
}

kmldom::StylePtr KmlUtils::createUniqueStyle(kmldom::Document* parent)
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::StylePtr style = factory->CreateStyle();
    bool isUnique = false;
    std::default_random_engine randomEndine;
    std::uniform_int_distribution<std::size_t> dist(0, 100000);
    std::string id;
    while (!isUnique) {
        std::size_t i = 0;
        isUnique = true;
        QString randomStr = "mcc_style_" + QString::number(dist(randomEndine));
        id = randomStr.toStdString();
        while (i < parent->get_styleselector_array_size() && isUnique) {
            const kmldom::StyleSelectorPtr& styleSelector = parent->get_styleselector_array_at(i);
            i++;
            if (id == styleSelector->get_id()) {
                isUnique = false;
            }
        }
    }
    style->set_id(id);
    parent->add_styleselector(style);
    return style;
}

kmldom::StylePtr KmlUtils::createUniqueStyleFor(kmldom::Document* parent, kmldom::Placemark* placemark)
{
    kmldom::StylePtr style = createUniqueStyle(parent);
    std::string url = "#";
    url.append(style->get_id());
    placemark->set_styleselector(style);
    placemark->set_styleurl(url);
    return style;
}

kmldom::PlacemarkPtr KmlUtils::createEmptyPoint(kmldom::Document* parent, const kmlbase::Vec3& pos)
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::PlacemarkPtr placemark = factory->CreatePlacemark();
    placemark->set_name("Новая точка");
    kmldom::PointPtr point = factory->CreatePoint();
    kmldom::CoordinatesPtr coordinates = factory->CreateCoordinates();
    coordinates->add_vec3(pos);
    point->set_coordinates(coordinates);
    placemark->set_geometry(point);

    kmldom::StylePtr style = createUniqueStyleFor(parent, placemark.get());
    kmldom::IconStylePtr iconStyle = factory->CreateIconStyle();
    iconStyle->set_scale(1);
    iconStyle->set_color(kmlColorFromQcolor(Qt::red));
    style->set_iconstyle(iconStyle);
    kmldom::LabelStylePtr labelStyle = factory->CreateLabelStyle();
    labelStyle->set_scale(1);
    labelStyle->set_color(kmlColorFromQcolor(Qt::white));
    style->set_labelstyle(labelStyle);
    return placemark;
}

kmldom::PlacemarkPtr KmlUtils::createEmptyPolyline(kmldom::Document* parent)
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::PlacemarkPtr placemark = factory->CreatePlacemark();
    placemark->set_name("Новая ломаная");
    kmldom::LineStringPtr lineString = factory->CreateLineString();
    kmldom::CoordinatesPtr coordinates = factory->CreateCoordinates();
    lineString->set_coordinates(coordinates);
    placemark->set_geometry(lineString);

    kmldom::StylePtr style = createUniqueStyleFor(parent, placemark.get());
    kmldom::LineStylePtr lineStyle = factory->CreateLineStyle();
    lineStyle->set_color(kmlColorFromQcolor(Qt::white));
    lineStyle->set_width(2);
    style->set_linestyle(lineStyle);

    return placemark;
}

kmldom::PlacemarkPtr KmlUtils::createEmptyPolygon(kmldom::Document* parent)
{
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::PlacemarkPtr placemark = factory->CreatePlacemark();
    placemark->set_name("Новый полигон");
    kmldom::PolygonPtr polygon = factory->CreatePolygon();
    kmldom::OuterBoundaryIsPtr outer = factory->CreateOuterBoundaryIs();
    kmldom::LinearRingPtr linearRing = factory->CreateLinearRing();
    kmldom::CoordinatesPtr coordinates = factory->CreateCoordinates();
    linearRing->set_coordinates(coordinates);
    outer->set_linearring(linearRing);
    polygon->set_outerboundaryis(outer);
    placemark->set_geometry(polygon);

    kmldom::StylePtr style = createUniqueStyleFor(parent, placemark.get());
    kmldom::LineStylePtr lineStyle = factory->CreateLineStyle();
    lineStyle->set_color(kmlColorFromQcolor(Qt::white));
    lineStyle->set_width(2);
    style->set_linestyle(lineStyle);

    kmldom::PolyStylePtr polyStyle = factory->CreatePolyStyle();
    polyStyle->set_color(kmlColorFromQcolor(Qt::white));
    style->set_polystyle(polyStyle);

    return placemark;
}

kmldom::FolderPtr KmlUtils::createEmptyDirectory(kmldom::Document* parent)
{
    (void)parent;
    kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
    kmldom::FolderPtr folder = factory->CreateFolder();
    folder->set_name("Новая папка");
    return folder;
}

QColor KmlUtils::qcolorFromKmlColor(const kmlbase::Color32& color)
{
    QColor qcolor;
    qcolor.setAlpha(color.get_alpha());
    qcolor.setRed(color.get_red());
    qcolor.setBlue(color.get_blue());
    qcolor.setGreen(color.get_green());
    return qcolor;
}

QColor KmlUtils::qcolorFromKmlColorInverted(const kmlbase::Color32& color)
{
    QColor qcolor;
    qcolor.setAlpha(color.get_alpha());
    qcolor.setRed(255 - color.get_red());
    qcolor.setBlue(255 - color.get_blue());
    qcolor.setGreen(255 - color.get_green());
    return qcolor;
}

kmldom::DocumentPtr KmlUtils::parentDocument(kmldom::Element* element)
{
    if (element->IsA(kmldom::Type_Document)) {
        return kmldom::AsDocument(element);
    }
    kmldom::ElementPtr parent = element->GetParent();
    if (parent) {
        return parentDocument(parent.get());
    }
    return nullptr;
}

kmldom::ContainerPtr KmlUtils::parentContainer(kmldom::Element* element)
{
    kmldom::ElementPtr parent = element->GetParent();
    if (parent && parent->IsA(kmldom::Type_Container)) {
        return kmldom::AsContainer(parent);
    }
    return nullptr;
}

bool KmlUtils::isEditableElement(const kmldom::Element* element)
{
    return element->IsA(kmldom::Type_Placemark) || element->IsA(kmldom::Type_Container);
}

std::size_t KmlUtils::countElementsWithType(const kmldom::Container* container, kmldom::KmlDomType type)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
        if (container->get_feature_array_at(i)->IsA(type)) {
            count++;
        }
    }
    return count;
}

std::size_t KmlUtils::countEditableElements(const kmldom::Container* container)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < container->get_feature_array_size(); i++) {
        kmldom::FeaturePtr feature = container->get_feature_array_at(i);
        if (feature->IsA(kmldom::Type_Placemark) || feature->IsA(kmldom::Type_Container)) {
            count++;
        }
    }
    return count;
}

kmldom::FeaturePtr KmlUtils::findEditableElementWithIndex(const kmldom::Container* container, std::size_t index)
{
    std::size_t i = 0;
    std::size_t fi = 0;
    while (i < container->get_feature_array_size()) {
        if (isEditableElement(container->get_feature_array_at(i).get())) {
            if (fi == index) {
                return container->get_feature_array_at(i);
            }
            fi++;
        }
        i++;
    }
    return nullptr;
}
}
}
}
