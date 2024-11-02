#include "bezier.h"

// Constructor
BezierPath::BezierPath(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
    : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3) {}

// Calculate position on the curve
glm::vec3 BezierPath::calculatePosition(float t) const {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    glm::vec3 position = uuu * m_p0;
    position += 3 * uu * t * m_p1;
    position += 3 * u * tt * m_p2;
    position += ttt * m_p3;

    return position;
}

// Set control points
void BezierPath::setControlPoints(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    m_p0 = p0;
    m_p1 = p1;
    m_p2 = p2;
    m_p3 = p3;
}
glm::vec3 BezierPath::getP0() const { return m_p0; }
glm::vec3 BezierPath::getP1() const { return m_p1; }
glm::vec3 BezierPath::getP2() const { return m_p2; }
glm::vec3 BezierPath::getP3() const { return m_p3; }