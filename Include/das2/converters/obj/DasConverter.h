// das2: Improved DENG asset manager library
// licence: Apache, see LICENCE file
// file: DasConverter.h - header file for obj file convertion class
// author: Karl-Mihkel Ott

#pragma once

#include <das2/Api.h>

#include <cstddef>
#include <cstring>
#include <cmath>
#include <trs/Iterators.h>
#include <trs/Vector.h>

#include <das2/converters/obj/Data.h>
#include <das2/DasStructures.h>
#include <das2/IConverter.h>
#include <das2/Serializer.h>

#include <cstdint>
#include <unordered_set>


namespace das2 {
    namespace obj {
        struct UnifiedVertex {
            TRS::Vector3<float> positionVertex;
            TRS::Vector2<float> textureVertex;
            TRS::Vector3<float> normalVertex;

            bool operator==(const UnifiedVertex& _vt) const {
                return positionVertex == _vt.positionVertex && textureVertex == _vt.textureVertex && normalVertex == _vt.normalVertex;
            }
        };
    }
}

namespace std {
    template <>
    struct hash<TRS::Vector3<float>> {
        size_t operator()(const TRS::Vector3<float>& _vec) const {
            size_t seed = 0;
            std::hash<float> hasher;
            seed ^= hasher(_vec.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(_vec.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(_vec.third) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    }; 

    template <>
    struct hash<das2::obj::UnifiedVertex> {
        size_t operator()(const das2::obj::UnifiedVertex& _vt) const {
            size_t seed = 0;
            std::hash<float> hasher;
            const size_t uPrime = 0x9e3779b9;

            for (auto it = _vt.positionVertex.Begin(); it != _vt.positionVertex.End(); it++)
                seed ^= hasher(*it) + uPrime + (seed << 6) + (seed >> 2);
            for (auto it = _vt.normalVertex.Begin(); it != _vt.normalVertex.End(); it++)
                seed ^= hasher(*it) + uPrime + (seed << 6) + (seed >> 2);
            for (auto it = _vt.textureVertex.Begin(); it != _vt.textureVertex.End(); it++)
                seed ^= hasher(*it) + uPrime + (seed << 6) + (seed >> 2);
        
            return seed;
        };
    };
}

namespace das2 {
    namespace obj {
        
        class DAS2_API DasConverter : public IConverter {
            private:
                const Object& m_obj;
                std::unordered_map<TRS::Vector3<float>, uint32_t> m_mapNormals;
                std::vector<TRS::Vector3<float>> m_smoothenedNormals;
                std::vector<std::array<TRS::Point3D<uint32_t>, 3>> m_triangulizedFaces;

                std::unordered_map<UnifiedVertex, uint32_t> m_reindexMap;
                std::vector<TRS::Vector3<float>> m_reindexedVertexPositions;
                std::vector<TRS::Vector2<float>> m_reindexedUVPositions;
                std::vector<TRS::Vector3<float>> m_reindexedNormals;

                std::vector<uint32_t> m_indices;

            private:
                void _Convert();
                void _TriangulizeFace(const std::vector<TRS::Point3D<uint32_t>>& _face);
                void _SmoothenNormals(const Group& _group);
                void _ReduceNormalRedundancy(const Group& _group);
                void _Reindex();

            public:
                DasConverter(const Object& _obj, const BinString& _szAuthorName = "", const BinString& _szComment = "", uint8_t _uZLibLevel = 0);
        };

    }
}