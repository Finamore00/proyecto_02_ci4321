#include "tank.hpp"

#include <iostream>

#include "components/destroyable_component.hpp"
#include "../mesh/mesh.hpp"
#include "../mesh/geometry.hpp"
#include "../input/input.hpp"
#include "../physics/collision_primitives.hpp"
#include "../physics/collider_component.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "../../thirdparty/glm/gtx/string_cast.hpp"
#include "../../thirdparty/glm/gtx/vector_angle.hpp"

Tank::Tank(SceneObject &parent, gl_utils::shader_program &shader) {

    //The box will be the parent of all other shapes in the tank. So its parent must be set outside
    mesh = new Mesh(create_box(1.0f, 0.5f, 2.0f), shader);
    mesh->shaderMaterial.tint = glm::vec3(1.0f, 0.0f, 0.0f);
    transform.set_parent(&parent.transform, false);
    active = true;

    //Create sphere geometry for the turret
    SceneObject *turret_sphere = new SceneObject;
    turret_sphere->transform.set_parent(&transform, true);
    turret_sphere->transform.set_local_position(glm::vec3(0.0f, 0.25f, 0));
    turret_sphere->transform.set_local_euler_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
    turret_transform = &turret_sphere->transform;
    transform.update_transform();

    turret_sphere->mesh = new Mesh(create_sphere(30, 20, 0.25), shader);
    turret_sphere->mesh->shaderMaterial.tint = glm::vec3(1.0f, 0.0f, 0.0f);
    turret_sphere->active = true;

    //Create pivot point for the cylinder
    SceneObject *cylinder_pivot = new SceneObject;
    cylinder_pivot->transform.set_parent(&turret_sphere->transform, true);
    cylinder_pivot->transform.set_local_position(glm::vec3(0.0, 0.0, 0.0));
    cylinder_pivot->transform.set_local_euler_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
    turret_transform->update_transform();
    cannon_pivot_transform = &cylinder_pivot->transform;

    //Create the actual turret cylinder
    SceneObject *turret_cylinder = new SceneObject;
    turret_cylinder->mesh = new Mesh(create_cylinder(30, 0.0625f, 1.0f), shader);
    turret_cylinder->mesh->shaderMaterial.tint = glm::vec3(1.0f, 0.0f, 0.0f);

    turret_cylinder->transform.set_parent(&cylinder_pivot->transform, true);
    turret_cylinder->transform.set_local_euler_rotation(glm::vec3(0.0f, 0.0f, 90.0f));
    turret_cylinder->transform.set_local_position(glm::vec3(0.5f, 0.0f, 0.0f));
    cylinder_pivot->transform.update_transform();
    turret_cylinder->active = true;

    //Bullet spawner
    SceneObject *bullet_spawner = new SceneObject;
    bullet_spawner->transform.set_parent(&turret_cylinder->transform, true);
    bullet_spawner->transform.set_local_euler_rotation(glm::vec3(90.0f, 0.0f, 0.0f));
    bullet_spawner->transform.set_local_position(glm::vec3(0.0f, -1.0f, 0.0f));
    turret_cylinder->transform.update_transform();
    spawner_transform = &bullet_spawner->transform;

    //Initiate bullets
    for (int i = 0; i < 3; i++) {
        bullets[i] = new Bullet(1.0f, -9.8f, true);
        SphereCollider* bullCol = new SphereCollider(bullets[i], .125f);
        bullets[i]->mesh = new Mesh(create_sphere(15, 10, 0.125f), shader);
        bullets[i]->mesh->shaderMaterial.tint = glm::vec3(0.0f, 0.0f, 1.0f);
        bullets[i]->mesh->shaderMaterial.ambient = glm::vec3(0.5f);
        bullets[i]->transform.set_parent(&parent.transform, false);
        bullets[i]->add_component(*bullCol);
        bullets[i]->add_component(*new DestroyableComponent(bullets[i]));
        bullets[i]->active = false;
    }
}


void Tank::update(float time) {
    rotate_turret(time);
    fire_bullet();
    rotate_tank(time);
    move(time);

    transform.update_transform();
    return;
}


void Tank::rotate_tank(float time) {
    InputManager *input;
    input = input->get_instance();
    const float pi = glm::pi<float>();

    glm::quat current_rotation = transform.get_world_rotation();
    glm::vec3 up = glm::normalize(transform.get_up_vector());

    glm::quat rotation;

    if (input->key_is_pressed(GLFW_KEY_A)) {
        rotation = glm::angleAxis(pi * time, up);
    } else if (input->key_is_pressed(GLFW_KEY_D)) {
        rotation = glm::angleAxis(-pi * time, up);
    } else {
        return;
    }

    rotation = glm::normalize(rotation);

    transform.set_world_rotation(current_rotation * rotation);
}   


//Turret rotates kinda weird rn
void Tank::rotate_turret(float time) {
    InputManager *input;
    input = input->get_instance();
    const float pi = glm::pi<float>();

    glm::quat turret_y_rotation = turret_transform->get_world_rotation();
    glm::quat pivot_x_rotation  = cannon_pivot_transform->get_world_rotation();
    glm::vec3 right = cannon_pivot_transform->get_front_vector();

    glm::quat rotation;
    float rotDir = 0.0f;
    if (input->key_is_pressed(GLFW_KEY_J)) {
        rotDir = 1.0f;
    } else if (input->key_is_pressed(GLFW_KEY_L)) {
        rotDir = -1.0f;
    }
    
    if (rotDir != 0.0f)
    {
        rotation = glm::angleAxis(pi * time * rotDir, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::normalize(rotation);
        turret_transform->set_world_rotation(rotation * turret_y_rotation);
        transform.update_transform();
    }

    rotDir = 0.0f;
    if (input->key_is_pressed(GLFW_KEY_I)) {
        rotDir = 1.0f;
    } else if (input->key_is_pressed(GLFW_KEY_K)) {
        rotDir = -1.0f;
    }

    float angBtw = glm::degrees(glm::angle(cannon_pivot_transform->get_right_vector(), glm::vec3(0.0f, 1.0f, 0.0f)));    
    if (rotDir != 0.0f)
    {
        if ((rotDir < 0.0f && (angBtw + glm::degrees(pi) * time * rotDir) < 90.0f) ||
            (rotDir > 0.0f && (angBtw - glm::degrees(pi) * time * rotDir) > 0.0f))
        {
            rotation = glm::angleAxis(pi * time * rotDir, right);
            rotation = glm::normalize(rotation);
            cannon_pivot_transform->set_world_rotation(rotation * pivot_x_rotation);
            transform.update_transform();
        }
    }
}


void Tank::move(float time) {
    InputManager *input;
    input = input->get_instance();

    glm::vec3 current_position = transform.get_world_position();
    glm::vec3 fwd = transform.get_front_vector();
    const float move_delta = 5.0f * time;  // Decide on a maximum speed

    if (input->key_is_pressed(GLFW_KEY_W)) {
        current_position += move_delta * fwd;
    }

    if (input->key_is_pressed(GLFW_KEY_S)) {
        current_position -= move_delta * fwd;
    }

    transform.set_world_position(current_position);
}


void Tank::fire_bullet() {
    InputManager *input;
    input = input->get_instance();

    bool any_key = input->key_is_pressed(GLFW_KEY_SPACE) || input->key_is_pressed(GLFW_KEY_LEFT_CONTROL);

    if (!any_key) {
        return;
    }

    float time = glfwGetTime();
    if (time - last_fired_time > 1.0f) {
        last_fired_time = time;

        //Look for first disabled bullet
        int i = 0;
        for (; i < 3 && bullets[i]->active; i++) {}

        if (i >= 3)
            i = last_fired_bullet;

        bool use_gravity = input->key_is_pressed(GLFW_KEY_SPACE);

        transform.update_transform();
        bullets[i]->spawn(*spawner_transform, 10.0f, use_gravity);
        bullets[i]->active = true;
        last_fired_bullet = (last_fired_bullet + 1) % 3;
    }
}

void Tank::update_bullets(float time) {
    for (int i = 0; i < 3; i++) {
        if (bullets[i]->active) {
            bullets[i]->update(time);
        }
    }
    transform.update_transform();
}