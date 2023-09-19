// das2: Improved DENG asset manager library
// licence: Apache, see LICENCE file
// file: DasStructures.h - header of C++ structures for das2
// author: Karl-Mihkel Ott

#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <ostream>
#include <variant>

#include <das2/Api.h>
#include <cvar/SID.h>
#include <trs/Vector.h>
#include <trs/Matrix.h>
#include <trs/Quaternion.h>

#define DAS2_MAGIC 0x0000000032736164

namespace das2 {

    class BinString {
        private:
            uint16_t m_uLength = 0;
            cvar::hash_t m_hshString = 0;
            char* m_pData = nullptr;

        public:
            BinString() = default;
            ~BinString() {
                delete[] m_pData;
            }

            BinString(const char* _szData) {
                if (_szData != nullptr) {
                    m_uLength = static_cast<uint16_t>(std::strlen(_szData));
                    m_pData = new char[m_uLength+1]{};
                    std::memcpy(m_pData, _szData, m_uLength);
                    m_hshString = RUNTIME_CRC(m_pData);
                }
            }

            BinString(const std::string& _str) {
                if (_str.size()) {
                    m_uLength = static_cast<uint16_t>(_str.size());
                    m_pData = new char[m_uLength+1]{};
                    std::memcpy(m_pData, _str.data(), _str.size());
                    m_hshString = RUNTIME_CRC(m_pData);
                }
            }

            BinString(const BinString& _str) :
                m_uLength(_str.m_uLength),
                m_hshString(_str.m_hshString)
            {
                if (m_uLength) {
                    m_pData = new char[m_uLength+1]{};
                    std::memcpy(m_pData, _str.m_pData, m_uLength);
                }
            }

            BinString(BinString&& _str) noexcept :
                m_uLength(_str.m_uLength),
                m_pData(_str.m_pData),
                m_hshString(_str.m_hshString)
            {
                _str.m_uLength = 0;
                _str.m_pData = nullptr;
                _str.m_hshString = 0;
            }

            BinString& operator=(const BinString& _other) {
                m_uLength = _other.m_uLength;
                m_hshString = _other.m_hshString;
                
                if (m_uLength) {
                    m_pData = new char[m_uLength+1]{};
                    std::memcpy(m_pData, _other.m_pData, m_uLength);
                }
                return *this;
            }

            inline bool operator==(const BinString& _str) const {
                return m_hshString == _str.m_hshString;
            }

            inline bool operator!=(const BinString& _str) const {
                return m_hshString != _str.m_hshString;
            }

            inline const char* CString() const {
                return m_pData;
            }

            inline uint16_t Length() const {
                return m_uLength;
            }

            inline cvar::hash_t Hash() const {
                return m_hshString;
            }

            std::ostream& operator<<(std::ostream& _stream);

    };
}


namespace std {
    template<>
    struct hash<das2::BinString> {
        bool operator()(const das2::BinString& _str) const {
            return _str.Hash();
        }
    };
}

namespace das2 {

    enum StructureIdentifier : char {
        StructureIdentifier_Unknown = 0x00,
        StructureIdentifier_Buffer = 0x01,
        StructureIdentifier_Mesh = 0x02,
        StructureIdentifier_MorphTarget = 0x03,
        StructureIdentifier_MeshGroup = 0x04,
        StructureIdentifier_Node = 0x05,
        StructureIdentifier_Scene = 0x06,
        StructureIdentifier_SkeletonJoint = 0x07,
        StructureIdentifier_Skeleton = 0x08,
        StructureIdentifier_Animation = 0x09,
        StructureIdentifier_AnimationChannel = 0x0a,
        StructureIdentifier_MaterialPhong = 0x0b,
        StructureIdentifier_MaterialPbr = 0x0c
    };

    enum MaterialType : char {
        MaterialType_Unknown,
        MaterialType_Phong,
        MaterialType_Pbr
    };

    enum AnimationTarget : char {
        AnimationTarget_Unknown,
        AnimationTarget_Weights,
        AnimationTarget_Translation,
        AnimationTarget_Rotation,
        AnimationTarget_Scale
    };

    enum InterpolationType : char {
        InterpolationType_Unknown,
        InterpolationType_Linear,
        InterpolationType_Step,
        InterpolationType_CubicSpline
    };


    class DAS2_API Header {
        private:
            uint64_t m_uMagic = 0;

        public:
            BinString szAuthorName = "";
            BinString szComment = "";
            uint32_t uVerticesCount = 0;
            uint32_t uMeshCount = 0;
            uint32_t uAnimationCount = 0;
            uint32_t uDefaultSceneIndex = 0;
            uint8_t bZlibLevel = 0;

        public:
            Header() = default;
            Header(const Header& _header);
            Header(Header&& _header);

            inline void Initialize() {
                m_uMagic = DAS2_MAGIC;
            }
            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Buffer {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;
            uint32_t m_uLength = 0;
            char* m_pData = nullptr;

        public:
            Buffer() = default;
            Buffer(const Buffer& _buffer);
            Buffer(Buffer&& _buffer);
            ~Buffer();

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Buffer;
            }

            inline char* Get(uint32_t _uOffset = 0) {
                return m_pData + _uOffset;
            }

            template<typename T = char>
            T* Get(uint32_t _uOffset = 0) {
                return static_cast<T*>(m_pData + _uOffset);
            }
            
            template <typename T = char>
            inline const T* Get(uint32_t _uOffset = 0) const {
                return static_cast<const T*>(m_pData + _uOffset);
            }

            template <typename InputIt>
            uint32_t PushRange(InputIt _first, InputIt _last) {
                std::size_t uDistance = std::distance(_first, _last);
                
                const uint32_t uOffset = m_uLength;
                m_uLength += static_cast<uint32_t>(uDistance);
                m_pData = static_cast<char*>(std::realloc(m_pData, m_uLength));

                if (!m_pData)
                    throw std::bad_alloc();

                size_t i = 0;
                for (auto it = _first; it != _last; it++, i++) {
                    static_cast<decltype(*it)*>(m_pData + uOffset)[i] = *it;
                }

                return uOffset;
            }

            template <typename T>
            uint32_t PushRange(const T* _pData, size_t _uLength) {
                const uint32_t uOffset = m_uLength;
                m_uLength += static_cast<uint32_t>(_uLength);
                m_pData = static_cast<char*>(std::realloc(m_pData, m_uLength));
                if (!m_pData)
                    throw std::bad_alloc();

                for (size_t i = 0; i < _uLength; i++)
                    static_cast<T*>(m_pData + uOffset)[i] = _pData[i];
            }

            inline uint32_t Size() const {
                return m_uLength;
            }
            
            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API MorphTarget {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;
            
        public:
            uint32_t uIndexBufferId = static_cast<uint32_t>(-1);
            uint32_t uIndexBufferOffset = 0;
            uint32_t uPositionVertexBufferId = static_cast<uint32_t>(-1);
            uint32_t uPositionVertexBufferOffset = 0;
            uint32_t uSurfaceNormalBufferId = static_cast<uint32_t>(-1);
            uint32_t uSurfaceNormalBufferOffset = 0;
            std::array<uint32_t, 8> arrUVBufferIds;
            std::array<uint32_t, 8> arrUVBufferOffsets;
            uint32_t uColorMultiplierId = static_cast<uint32_t>(-1);
            uint32_t uColorMultiplierOffset = 0;

        public:
            MorphTarget() = default;
            MorphTarget(const MorphTarget& _morphTarget) = default;

            inline void Initialize() {
                m_bStructure = StructureIdentifier_MorphTarget;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Mesh {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;
        
        public:
            uint32_t uIndexBufferId = static_cast<uint32_t>(-1);
            uint32_t uIndexBufferOffset = 0;
            uint32_t uDrawCount = 0;
            uint32_t uPositionVertexBufferId = static_cast<uint32_t>(-1);
            uint32_t uPositionVertexBufferOffset = 0;
            uint32_t uSurfaceNormalBufferId = static_cast<uint32_t>(-1);
            uint32_t uSurfaceNormalBufferOffset = 0;
            std::array<uint32_t, 8> arrUVBufferIds;
            std::array<uint32_t, 8> arrUVBufferOffsets;
            uint32_t uColorMultiplierId = static_cast<uint32_t>(-1);
            uint32_t uColorMultiplierOffset = 0;
            std::array<uint32_t, 8> arrSkeletalJointIndexBufferIds;
            std::array<uint32_t, 8> arrSkeletalJointIndexBufferOffsets;
            std::array<uint32_t, 8> arrSkeletalJointWeightBufferIds;
            std::array<uint32_t, 8> arrSkeletalJointWeightBufferOffsets;
            MaterialType bMaterialType = MaterialType_Unknown;
            uint32_t uMaterialId = static_cast<uint32_t>(-1);
            std::vector<MorphTarget> morphTargets;
            std::vector<Mesh> multipleLods;

        public:
            Mesh() = default;
            Mesh(const Mesh& _mesh) = default;
            Mesh(Mesh&& _mesh) = default;
            ~Mesh();

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Mesh;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API MeshGroup {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr; 
            std::vector<uint32_t> meshes;

        public:
            MeshGroup() = default;
            MeshGroup(const MeshGroup& _meshGroup) = default;
            MeshGroup(MeshGroup&& _meshGroup) = default;

            inline void Initialize() {
                m_bStructure = StructureIdentifier_MeshGroup;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Node {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr;
            uint32_t uChildrenCount = 0;
            uint32_t* pChildren = nullptr;
            uint32_t uMeshGroupId = static_cast<uint32_t>(-1);
            uint32_t uSkeletonId = static_cast<uint32_t>(-1);
            TRS::Matrix4<float> mCustomTransform;
            TRS::Quaternion qRotation;
            TRS::Vector3<float> vTranslation;
            float fScale = 1.f;

        public:
            Node() = default;
            Node(const Node& _node) = default;
            Node(Node&& _node) = default;

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Node;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Scene {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr;
            std::vector<uint32_t> rootNodes;

        public:
            Scene() = default;
            Scene(const Scene& _scene);
            Scene(Scene&& _scene);

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Scene;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API SkeletonJoint {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr;
            std::vector<uint32_t> children;
            TRS::Matrix4<float> mInverseBindPos;
            TRS::Quaternion qRotation;
            TRS::Vector3<float> vTranslation;
            float fScale = 1.f;

        public:
            SkeletonJoint() = default;
            SkeletonJoint(const SkeletonJoint& _skeletonJoint);
            SkeletonJoint(SkeletonJoint&& _skeletonJoint);
    
            inline void Initialize() {
                m_bStructure = StructureIdentifier_SkeletonJoint;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Skeleton {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr;
            uint32_t uParent = static_cast<uint32_t>(-1);
            std::vector<uint32_t> joints;

        public:
            Skeleton() = default;
            Skeleton(const Skeleton& _skeleton) = default;
            Skeleton(Skeleton&& _skeleton) = default;

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Skeleton;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API Animation {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = nullptr;
            std::vector<uint32_t> animationChannels;

        public:
            Animation() = default;
            Animation(const Animation& _animation);
            Animation(Animation&& _animation);

            inline void Initialize() {
                m_bStructure = StructureIdentifier_Animation;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API AnimationChannel {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;
            using _Variant = std::variant<std::vector<float>, TRS::Vector3<float>, TRS::Quaternion, float>;

        public:
            uint32_t uNodePropertyId = static_cast<uint32_t>(-1);
            uint32_t uJointPropertyId = static_cast<uint32_t>(-1);
            AnimationTarget bAnimationTarget = AnimationTarget_Unknown;
            InterpolationType bInterpolationType = InterpolationType_Unknown;
            uint32_t uKeyframeCount = 0;
            uint32_t uWeightCount = 0;
            std::vector<float> keyframes;
            std::vector<std::array<_Variant, 3>> tangents;
            std::vector<_Variant> targetValues;

        public:
            AnimationChannel() = default;
            AnimationChannel(const AnimationChannel& _animationChannel);
            AnimationChannel(AnimationChannel& _animationChannel);

            inline void Initialize() {
                m_bStructure = StructureIdentifier_AnimationChannel;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API MaterialPhong {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = "";
            TRS::Vector4<float> vDiffuse = { 0.f, 0.f, 0.f, 1.f };
            TRS::Vector4<float> vSpecular = { 0.f, 0.f, 0.f, 1.f };
            TRS::Vector4<float> vEmission = { 0.f, 0.f, 0.f, 1.f };
            BinString szDiffuseMapUri = "";
            BinString szSpecularMapUri = "";
            BinString szEmissionMapUri = "";

        public:
            MaterialPhong() = default;
            MaterialPhong(const MaterialPhong& _materialPhong);
            MaterialPhong(MaterialPhong&& _materialPhong);

            inline void Initialize() {
                m_bStructure = StructureIdentifier_MaterialPhong;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };


    class DAS2_API MaterialPbr {
        private:
            StructureIdentifier m_bStructure = StructureIdentifier_Unknown;

        public:
            BinString szName = "";
            TRS::Vector4<float> vAlbedoFactor = { 1.f, 1.f, 1.f, 1.f };
            TRS::Vector4<float> vEmissiveFactor = { 0.f, 0.f, 0.f, 1.f };
            float fRoughness = 0.f;
            float fMetallic = 0.f;
            float fAmbientOcclusion = 1.f;
            BinString szAlbedoMapUri = "";
            BinString szEmissionMapUri = "";
            BinString szRoughnessMapUri = "";
            BinString szMetallicMapUri = "";
            BinString szAmbientOcclusionMapUri = "";
        
        public:
            MaterialPbr() = default;
            MaterialPbr(const MaterialPbr& _materialPbr);
            MaterialPbr(MaterialPbr&& _materialPbr);

            inline void Initialize() {
                m_bStructure = StructureIdentifier_MaterialPbr;
            }

            std::ostream& operator<<(std::ostream& _stream);
    };

    struct Model {
        Model() = default;
        Model(const Model& _model);
        Model(Model&& _model);

        Header header;
        Buffer buffer;
        std::vector<Mesh> meshes;
        std::vector<MeshGroup> meshGroups;
        std::vector<Node> nodes;
        std::vector<Scene> scenes;
        std::vector<SkeletonJoint> skeletonJoints;
        std::vector<Skeleton> skeletons;
        std::vector<Animation> animations;
        std::vector<AnimationChannel> animationChannels;
        std::vector<MaterialPhong> phongMaterials;
        std::vector<MaterialPbr> pbrMaterials;
    };

}
