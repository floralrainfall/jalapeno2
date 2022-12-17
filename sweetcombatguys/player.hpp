#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <scene.hpp>
#include <jpbsp.hpp>

namespace SCG
{
    class Player : public Scene::MeshNode
    {
    public:
        bool local_player;
        glm::vec3 acceleration;
        JPBSP::BSPSceneNode* map;

        Player(SceneNode* parent);

        virtual void Update();
    };
};

#endif