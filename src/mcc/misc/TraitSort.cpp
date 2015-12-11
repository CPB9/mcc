/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mcc/misc/TraitSort.h"

namespace mcc {
    namespace misc {

        std::vector<NodeLevel> sortTraits(const TraitDescriptionList& traits)
        {
            std::map<size_t, uint16_t> nodeToIndex;
            for (int i = 0; i < traits.size(); ++i)
            {
                nodeToIndex[traits[i]._id] = i;
            }

            std::vector<NodeLevel> levels;
            for (int i = 0; i < traits.size(); ++i)
            {
                const auto& trait = traits[i];

                size_t item = trait._id;
                size_t parent = trait._parentId;
                uint8_t level = 0;
                while (parent != item)
                {
                    const auto& j = nodeToIndex.find(parent);
                    if (j == nodeToIndex.end())
                        break;
                    item = parent;
                    parent = traits[j->second]._parentId;
                    ++level;
                }
                levels.emplace_back(NodeLevel((uint16_t)trait._id, level, i));
            }

            std::sort(levels.begin(), levels.end()
                , [](const NodeLevel& r, const NodeLevel& l)
            {
                if (r.level != l.level)
                    return r.level < l.level;
                return r.node > l.node;
            }
            );
            return levels;
        }

    }
}

