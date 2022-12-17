#include "jpbsp.hpp"
#include <math.h>
#include <float.h>

#define EPSILON 0.03125f

// http://web.archive.org/web/20111112060250/http://www.devmaster.net/articles/quake3collision/

glm::vec3 CollisionSwizzleVecs(float* vecs)
{
    glm::vec3 d;
    d.x = vecs[0];
    d.y = vecs[2];
    d.z = -vecs[1];
    return d;
}

void JPBSP::BSPFile::CheckBrush(BSPBrush* brush, float* start, float* end, TraceOutput* output)
{
    float startFraction = -1.0f;
    float endFraction = 1.0f;
    bool startsOut = false;
    bool endsOut = false;

    for(int i = 0; i < brush->n_brushsides; i++)
    {
        BSPBrushSide side = ((BSPBrushSide*)bsp_data[BET_BRUSHSIDES])[brush->brushside + i];
        BSPPlane plane = ((BSPPlane*)bsp_data[BET_PLANES])[side.plane];
        float s_dist, e_dist;
        if(output->type == TraceOutput::BSP_TRACETYPE_RAY)
        {
            s_dist = glm::dot(*(glm::vec3*)start,*(glm::vec3*)plane.normal) - plane.distance;
            e_dist = glm::dot(*(glm::vec3*)end,*(glm::vec3*)plane.normal) - plane.distance;
        }
        else if(output->type == TraceOutput::BSP_TRACETYPE_SPHERE)
        {
            s_dist = glm::dot(*(glm::vec3*)start,*(glm::vec3*)plane.normal) - (plane.distance + output->radius);
            e_dist = glm::dot(*(glm::vec3*)end,*(glm::vec3*)plane.normal) - (plane.distance + output->radius);
        }
        else if (output->type == TraceOutput::BSP_TRACETYPE_BOX)
        {
            float offset[3];
            for (int j = 0; j < 3; j++)
            {
                if (plane.normal[j] < 0)
                    offset[j] = output->maxs[j];
                else
                    offset[j] = output->mins[j];
            }

            s_dist =    (start[0] + offset[0]) * plane.normal[0] +
                        (start[1] + offset[1]) * plane.normal[1] +
                        (start[2] + offset[2]) * plane.normal[2] -
                        plane.distance;
            e_dist =    (end[0] + offset[0]) * plane.normal[0] +
                        (end[1] + offset[1]) * plane.normal[1] +
                        (end[2] + offset[2]) * plane.normal[2] -
                        plane.distance;
        }
        if (s_dist > 0)
            startsOut = true;
        if (e_dist > 0)
            endsOut = true;
        if (s_dist > 0 && e_dist > 0)
        {   
            return;
        }
        if (s_dist <= 0 && e_dist <= 0)
        {   
            continue;
        }
        if (s_dist > e_dist)
        {   // line is entering into the brush
            float fraction = (s_dist - EPSILON) / (s_dist - e_dist);  // *
            if (fraction > startFraction)
                startFraction = fraction;
        }
        else
        {   // line is leaving the brush
            float fraction = (s_dist + EPSILON) / (s_dist - e_dist);  // *
            if (fraction < endFraction)
                endFraction = fraction;
        }
    }

    if (startsOut == false)
    {
        output->starts_out = false;
        if (endsOut == false)
            output->all_solid = true;
        return;
    }

    if (startFraction < endFraction)
    {
        if (startFraction > -1 && startFraction < output->fraction)
        {
            if (startFraction < 0)
                startFraction = 0;
            output->fraction = startFraction;
        }
    }
}

void JPBSP::BSPFile::CheckNode(int node_idx, float start_fraction, float end_fraction, float* start, float* end, TraceOutput* output)
{
    if (node_idx < 0)
    {   // this is a leaf
        BSPLeaf* leaf = &((BSPLeaf*)bsp_data[BET_LEAFS])[-(node_idx + 1)];
        for (int i = 0; i < leaf->n_leafbrushes; i++)
        {
            BSPBrush* brush = &((BSPBrush*)bsp_data[BET_BRUSHES])[leaf->leafbrush + i];
            if (brush->n_brushsides > 0)
            {
                BSPTexture* texture = &((BSPTexture*)bsp_data[BET_TEXTURES])[brush->texture];
                if(texture->contents & 1)
                    CheckBrush( brush, start, end, output );
            }
        }

        // don't have to do anything else for leaves
        return;
    }

    BSPNode node = ((BSPNode*)bsp_data[BET_NODES])[node_idx];
    BSPPlane plane = ((BSPPlane*)bsp_data[BET_PLANES])[node.plane];
    float s_dist = glm::dot(*(glm::vec3*)start,*(glm::vec3*)plane.normal) - plane.distance;
    float e_dist = glm::dot(*(glm::vec3*)end,*(glm::vec3*)plane.normal) - plane.distance;
    float offset = 0.f;

    if(output->type == TraceOutput::BSP_TRACETYPE_RAY)
    {
        offset = 0.f;
    }
    else if(output->type == TraceOutput::BSP_TRACETYPE_SPHERE)
    {
        offset = output->radius;
    }
    else if (output->type == TraceOutput::BSP_TRACETYPE_BOX)
    {
        // this is just a dot product, but we want the absolute values
        offset = (float)(fabs( output->xtnts[0] * plane.normal[0] ) +
                         fabs( output->xtnts[1] * plane.normal[1] ) +
                         fabs( output->xtnts[2] * plane.normal[2] ) );
    }

    if (s_dist >= offset && e_dist >= offset)
    {   // both points are in front of the plane
        // so check the front child
        CheckNode(node.children[0], start_fraction, end_fraction, start, end, output);
    }
    else if (s_dist < -offset && e_dist < -offset)
    {   // both points are behind the plane
        // so check the back child
        CheckNode(node.children[1], start_fraction, end_fraction, start, end, output);
    }
    else
    {
        int side;
        float fraction1, fraction2, middleFraction;
        float middle[3];

        // STEP 1: split the segment into two
        if (s_dist < e_dist)
        {
            side = 1; // back
            float inverseDistance = 1.0f / (s_dist - e_dist);
            fraction1 = (s_dist - offset + EPSILON) * inverseDistance;
            fraction2 = (s_dist + offset + EPSILON) * inverseDistance;
        }
        else if (e_dist < s_dist)
        {
            side = 0; // front
            float inverseDistance = 1.0f / (s_dist - e_dist);
            fraction1 = (s_dist + offset + EPSILON) * inverseDistance;
            fraction2 = (s_dist - offset - EPSILON) * inverseDistance;
        }
        else
        {
            side = 0; // front
            fraction1 = 1.0f;
            fraction2 = 0.0f;
        }

        // STEP 2: make sure the numbers are valid
        if (fraction1 < 0.0f) fraction1 = 0.0f;
        else if (fraction1 > 1.0f) fraction1 = 1.0f;
        if (fraction2 < 0.0f) fraction2 = 0.0f;
        else if (fraction2 > 1.0f) fraction2 = 1.0f;

        // STEP 3: calculate the middle point for the first side
        middleFraction = start_fraction +
                         (end_fraction - start_fraction) * fraction1;
        for (int i = 0; i < 3; i++)
            middle[i] = start[i] + fraction1 * (end[i] - start[i]);

        // STEP 4: check the first side
        CheckNode( node.children[side], start_fraction, end_fraction,
                   start, middle, output );

        // STEP 5: calculate the middle point for the second side
        middleFraction = start_fraction +
                        (end_fraction - end_fraction) * fraction2;
        for (int i = 0; i < 3; i++)
            middle[i] = start[i] + fraction2 * (end[i] - start[i]);

        // STEP 6: check the second side
        CheckNode( node.children[!side], middleFraction, end_fraction,
                   middle, end, output );  
    }
}

JPBSP::TraceOutput JPBSP::BSPFile::Trace(float* v_start, float* v_end, TraceOutput* output)
{
    output->starts_out = true;
    output->all_solid = false;
    output->fraction = 1.f;

    CheckNode(0, 0.f, 1.f, v_start, v_end, output);

    if(output->fraction == 1.f)
    {
        memcpy(output->end, v_end, 3 * sizeof(float));
    }
    else
    {
        for(int i = 0; i < 3; i++)
        {
            output->end[i] = v_start[i] * output->fraction * (v_end[i] - v_start[i]);
        }
    }

    return *output;
}

JPBSP::TraceOutput JPBSP::BSPFile::TraceRay(float* v_start, float* v_end)
{
    glm::vec3 s_start = CollisionSwizzleVecs(v_start);
    glm::vec3 s_end = CollisionSwizzleVecs(v_end);
    float* fp_start = (float*)&s_start;
    float* fp_end   = (float*)&s_end;
    TraceOutput output;
    output.type = TraceOutput::BSP_TRACETYPE_RAY;
    Trace(fp_start,fp_end,&output);
    return output;
}

JPBSP::TraceOutput JPBSP::BSPFile::TraceSphere(float* v_start, float* v_end, float radius)
{
    glm::vec3 s_start = CollisionSwizzleVecs(v_start);
    glm::vec3 s_end = CollisionSwizzleVecs(v_end);
    float* fp_start = (float*)&s_start;
    float* fp_end   = (float*)&s_end;
    TraceOutput output;
    output.type = TraceOutput::BSP_TRACETYPE_SPHERE;
    output.radius = radius;
    Trace(fp_start,fp_end,&output);
    return output;
}

JPBSP::TraceOutput JPBSP::BSPFile::TraceBox(float* v_start, float* v_end, float* v_mins, float* v_maxs)
{
    glm::vec3 s_start = CollisionSwizzleVecs(v_start);
    glm::vec3 s_end = CollisionSwizzleVecs(v_end);
    glm::vec3 s_mins = CollisionSwizzleVecs(v_mins);
    glm::vec3 s_maxs = CollisionSwizzleVecs(v_maxs);

    float* fp_start = (float*)&s_start;
    float* fp_end   = (float*)&s_end;
    float* fp_mins  = (float*)&s_mins;
    float* fp_maxs  = (float*)&s_maxs;

    if (fp_mins[0] == 0 && fp_mins[1] == 0 && fp_mins[2] == 0 &&
        fp_maxs[0] == 0 && fp_maxs[1] == 0 && fp_maxs[2] == 0)
    {   // the user called TraceBox, but this is actually a ray
        return TraceRay( fp_start, fp_end );
    }
    else
    {
        TraceOutput output;
        output.type = TraceOutput::BSP_TRACETYPE_BOX;
        memcpy(output.mins,fp_mins,sizeof(output.mins));
        memcpy(output.maxs,fp_maxs,sizeof(output.maxs));
        output.xtnts[0] = -fp_mins[0] > fp_maxs[0] ?
                          -fp_mins[0] : fp_maxs[0];
        output.xtnts[1] = -fp_mins[1] > fp_maxs[1] ?
                          -fp_mins[1] : fp_maxs[1];
        output.xtnts[2] = -fp_mins[2] > fp_maxs[2] ?
                          -fp_mins[2] : fp_maxs[2];
        Trace(v_start,v_end,&output);
        return output;
    }
}