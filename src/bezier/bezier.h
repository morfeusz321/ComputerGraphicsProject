#ifndef BEZIER_PATH_H
#define BEZIER_PATH_H

#include <glm/glm.hpp>

class BezierPath {
public:
    // Constructor to initialize control points
    BezierPath(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);

    // Function to calculate position on the curve
    glm::vec3 calculatePosition(float t) const;

    // Set new control points
    void setControlPoints(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);
    glm::vec3 getP0() const;
    glm::vec3 getP1() const;
    glm::vec3 getP2() const;
    glm::vec3 getP3() const;

private:
    // Control points
    glm::vec3 m_p0, m_p1, m_p2, m_p3;
};

#endif // BEZIER_PATH_H
