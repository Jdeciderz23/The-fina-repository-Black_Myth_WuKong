#include "Collider.h"
#include "3d/CCSprite3D.h"
#include "3d/CCMesh.h"
#include <fstream>
#include <sstream>
#include <algorithm>

USING_NS_CC;

/**
 * 创建地形碰撞器实例
 * @param terrainModel 关联的 3D 地形模型
 * @param objFilePath 可选的 .obj 模型文件路径，用于提取精确的碰撞网格
 * @return 碰撞器实例指针
 */
TerrainCollider* TerrainCollider::create(Sprite3D* terrainModel, const std::string& objFilePath) {
    auto pRet = new (std::nothrow) TerrainCollider();
    if (pRet && pRet->init(terrainModel, objFilePath)) {
        pRet->autorelease();
        return pRet;
    }
    CC_SAFE_DELETE(pRet);
    return nullptr;
}

/**
 * 初始化碰撞器
 * 逻辑：优先尝试从 .obj 文件加载精确三角形，如果失败则回退到基于 AABB 的简单碰撞
 */
bool TerrainCollider::init(Sprite3D* terrainModel, const std::string& objFilePath) {
    if (!terrainModel) return false;
    _terrain = terrainModel;
    _terrain->retain(); // 增加引用计数，防止模型被提前释放
    
    bool loaded = false;
    if (!objFilePath.empty()) {
        // 尝试从磁盘加载 .obj 文件
        loaded = loadFromObj(objFilePath);
    }

    // 如果没有路径或加载失败，生成一个基于 AABB 范围的平面作为碰撞体（保底逻辑）
    if (!loaded || _triangles.empty()) {
        extractTriangles(_terrain);
    }
    
    return true;
}

/**
 * 解析 .obj 文件以提取三角形面片
 * .obj 文件包含顶点 (v) 和面 (f) 信息
 */
bool TerrainCollider::loadFromObj(const std::string& objFilePath) {
    // 获取文件的完整路径（适配 Cocos2d-x 的资源管理）
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(objFilePath);
    std::string content = FileUtils::getInstance()->getStringFromFile(fullPath);
    if (content.empty()) return false;

    std::vector<Vec3> vertices; // 临时存储原始顶点
    std::stringstream ss(content);
    std::string line;
    
    // 获取当前模型的缩放和位置，以便将局部坐标转换为世界坐标
    float scale = _terrain->getScale();
    Vec3 pos = _terrain->getPosition3D();

    while (std::getline(ss, line)) {
        if (line.size() < 2) continue;

        // 解析顶点坐标：v x y z
        if (line[0] == 'v' && line[1] == ' ') {
            std::stringstream vss(line.substr(2));
            float x, y, z;
            if (vss >> x >> y >> z) {
                // 转换到世界空间坐标系
                vertices.push_back(Vec3(x * scale + pos.x, y * scale + pos.y, z * scale + pos.z));
            }
        } 
        // 解析面索引：f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
        else if (line[0] == 'f' && line[1] == ' ') {
            std::stringstream fss(line.substr(2));
            std::string v1_s, v2_s, v3_s;
            if (fss >> v1_s >> v2_s >> v3_s) {
                try {
                    // 处理可能带斜杠的索引（如 1/1/1 或 1//1）
                    auto parseIdx = [](const std::string& s) {
                        size_t p = s.find('/');
                        return (p == std::string::npos) ? std::stoi(s) : std::stoi(s.substr(0, p));
                    };

                    // .obj 索引从 1 开始，需减 1 匹配 vector 索引
                    int i1 = parseIdx(v1_s) - 1;
                    int i2 = parseIdx(v2_s) - 1;
                    int i3 = parseIdx(v3_s) - 1;

                    // 索引合法性检查，防止数组越界导致崩溃
                    if (i1 >= 0 && i1 < (int)vertices.size() &&
                        i2 >= 0 && i2 < (int)vertices.size() &&
                        i3 >= 0 && i3 < (int)vertices.size()) {
                        // 存储生成的三角形面片数据
                        _triangles.push_back({vertices[i1], vertices[i2], vertices[i3]});
                    }
                } catch (...) {
                    // 忽略格式错误的行
                }
            }
        }
    }
    return !_triangles.empty();
}

/**
 * 提取备用三角形（兜底方案）
 * 当无法解析模型文件时，使用模型的包围盒 (AABB) 底部作为一层水平碰撞面
 */
void TerrainCollider::extractTriangles(Sprite3D* model) {
    auto aabb = model->getAABB();
    Vec3 min = aabb._min;
    Vec3 max = aabb._max;
    float groundY = min.y; // 取包围盒底部高度
    
    // 创建两个三角形组成一个矩形平面
    _triangles.push_back({Vec3(min.x, groundY, min.z), Vec3(max.x, groundY, min.z), Vec3(max.x, groundY, max.z)});
    _triangles.push_back({Vec3(min.x, groundY, min.z), Vec3(max.x, groundY, max.z), Vec3(min.x, groundY, max.z)});
}

/**
 * 射线与地形所有三角形的求交检测
 * @param ray 射线（通常从角色脚部上方垂直向下发射）
 * @param hitDist 输出：射线起点到最近碰撞点的距离
 * @return 是否发生碰撞
 */
bool TerrainCollider::rayIntersects(const CustomRay& ray, float& hitDist) {
    float closestDist = FLT_MAX;
    bool hit = false;
    
    for (const auto& tri : _triangles) {
        // 性能优化：快速 AABB 过滤 (2D 投影)
        // 在进行复杂的射线-三角形数学计算前，先判断射线原点是否在三角形的 XZ 投影范围内
        float minX = std::min({tri.v0.x, tri.v1.x, tri.v2.x});
        float maxX = std::max({tri.v0.x, tri.v1.x, tri.v2.x});
        float minZ = std::min({tri.v0.z, tri.v1.z, tri.v2.z});
        float maxZ = std::max({tri.v0.z, tri.v1.z, tri.v2.z});
        
        // 如果射线在平面投影上都没碰到三角形的范围，直接跳过
        if (ray.origin.x < minX || ray.origin.x > maxX || ray.origin.z < minZ || ray.origin.z > maxZ) continue;

        float t;
        // 执行精确的 Möller-Trumbore 射线-三角形相交算法
        if (intersectTriangle(ray, tri, t)) {
            // 寻找距离最近的碰撞点（防止角色穿透多层地形）
            if (t < closestDist && t > 0) {
                closestDist = t;
                hit = true;
            }
        }
    }
    
    if (hit) hitDist = closestDist;
    return hit;
}

/**
 * 核心数学算法：Möller-Trumbore 射线-三角形相交检测
 * 此算法不需要计算平面方程，效率极高。
 * @param t 返回相交点在射线方向上的参数值（距离 = t * |direction|）
 */
bool TerrainCollider::intersectTriangle(const CustomRay& ray, const Triangle& tri, float& t) {
    Vec3 edge1 = tri.v1 - tri.v0;
    Vec3 edge2 = tri.v2 - tri.v0;
    Vec3 h, s, q;
    float a, f, u, v;
    
    Vec3::cross(ray.direction, edge2, &h);
    a = edge1.dot(h);
    
    // 如果 a 接近 0，说明射线与三角形平面平行
    if (a > -0.00001f && a < 0.00001f) return false;
    
    f = 1.0f / a;
    s = ray.origin - tri.v0;
    u = f * s.dot(h);
    
    // 检查重心坐标 u 是否在三角形内部
    if (u < 0.0f || u > 1.0f) return false;
    
    Vec3::cross(s, edge1, &q);
    v = f * ray.direction.dot(q);
    
    // 检查重心坐标 v 及 u+v 是否在三角形内部
    if (v < 0.0f || u + v > 1.0f) return false;
    
    // 计算射线参数 t
    t = f * edge2.dot(q);
    
    // 如果 t > 0，说明交点在射线正方向上
    return t > 0.00001f;
}
