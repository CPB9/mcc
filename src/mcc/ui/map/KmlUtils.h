/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

namespace kmlbase {
class Color32;
class Vec3;
}

#include <kml/dom/kml_ptr.h>
#include <kml/dom/kml22.h>

#include <QColor>

namespace mcc {
namespace ui {
namespace map {

class KmlUtils {
public:
    static kmldom::StylePtr createUniqueStyle(kmldom::Document* parent);
    static kmldom::StylePtr createUniqueStyleFor(kmldom::Document* parent, kmldom::Placemark* feature);

    static kmldom::StylePtr resolveStyle(const kmldom::Document* parent, const std::string& styleUrl);
    static kmldom::StylePtr resolveStyle(const kmldom::Document* parent, const kmldom::Feature* feature);
    static kmldom::CoordinatesPtr resolveCoordinates(const kmldom::Placemark* placemark);

    static kmlbase::Color32 kmlColorFromQcolor(const QColor& qcolor);
    static QColor qcolorFromKmlColor(const kmlbase::Color32& color);
    static QColor qcolorFromKmlColorInverted(const kmlbase::Color32& color);

    static kmldom::PlacemarkPtr createEmptyPoint(kmldom::Document* parent, const kmlbase::Vec3& pos);
    static kmldom::PlacemarkPtr createEmptyPolyline(kmldom::Document* parent);
    static kmldom::PlacemarkPtr createEmptyPolygon(kmldom::Document* parent);
    static kmldom::FolderPtr createEmptyDirectory(kmldom::Document* parent);

    static kmldom::DocumentPtr parentDocument(kmldom::Element* element);
    static kmldom::ContainerPtr parentContainer(kmldom::Element* element);
    static bool isEditableElement(const kmldom::Element* element);

    static std::size_t countElementsWithType(const kmldom::Container* container, kmldom::KmlDomType type);
    static std::size_t countEditableElements(const kmldom::Container* container);
    static kmldom::FeaturePtr findEditableElementWithIndex(const kmldom::Container* container, std::size_t index);
    // static void setDefaultIconStyle(kmldom::PlacemarkPtr*);
};
}
}
}
