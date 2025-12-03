#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "hh.h"

#include <stdbool.h>

struct pose {
    double timestamp;
    size_t frame;
    double x, y, z, q[4];
};

int
main(void) {
    // build path to odometry.csv
    char* path = path_alloc(PROJECT_ROOT);
    ASSERT(path_join(path, "tests", "assets", "odom.csv"), "Failed construct path to odom.csv");
    DBG("Path to odometry: %s", path);
    // read contents
    char* contents = read_entire_file(path);
    path_free(path);
    ASSERT(contents != NULL, "Failed to read odom.csv");
    span_t span;
    ASSERT(span_init(&span, contents, ","), "Failed to initialize span");
    struct pose* poses = NULL, pose;
    // skip header
    ASSERT(span_next_line(&span), "Failed to skip header line");
    // parse all poses
    for(bool flag = true; flag;) {
        ASSERT(span_parse_next(&span, "%lf", &pose.timestamp), "Failed to parse timestamp: %lf", pose.timestamp);
        ASSERT(span_parse_next(&span, "%zu", &pose.frame), "Failed to parse frame: %zu", pose.frame);
        ASSERT(span_parse_next(&span, "%lf", &pose.x), "Failed to parse x: %lf", pose.x);
        ASSERT(span_parse_next(&span, "%lf", &pose.y), "Failed to parse y: %lf", pose.y);
        ASSERT(span_parse_next(&span, "%lf", &pose.z), "Failed to parse z: %lf", pose.z);
        ASSERT(span_parse_next(&span, "%lf", &pose.q[0]), "Failed to parse qx: %lf", pose.q[0]);
        ASSERT(span_parse_next(&span, "%lf", &pose.q[1]), "Failed to parse qy: %lf", pose.q[1]);
        ASSERT(span_parse_next(&span, "%lf", &pose.q[2]), "Failed to parse qz: %lf", pose.q[2]);
        if(!span_parse_next(&span, "%lf", &pose.q[3])) flag = false;
        // DBG("timestamp = %.2lf, frame = %zu, x = %.2lf, y = %.2lf, z = %.2lf, q = [%.2lf, %.2lf, %.2lf, %.2lf]", 
        //     pose.timestamp, pose.frame, pose.x, pose.y, pose.z, pose.q[0], pose.q[1], pose.q[2], pose.q[3]);
        darrput(poses, pose);
    }
    // calculate average position
    double pose_avg[3] = { 0.0, 0.0, 0.0 };
    for(size_t i = 0; i < darrlen(poses); ++i) {
        pose_avg[0] += poses[i].x;
        pose_avg[1] += poses[i].y;
        pose_avg[2] += poses[i].z;
    }
    pose_avg[0] /= (double) darrlen(poses);
    pose_avg[1] /= (double) darrlen(poses);
    pose_avg[2] /= (double) darrlen(poses);
    DBG("avg. pos. of %zu odom. entries: x = %.2lf, y = %.2lf, z = %.2lf", darrlen(poses), pose_avg[0], pose_avg[1], pose_avg[2]);
    // print first and last pose
    pose = poses[0];
    DBG("first entry: timestamp = %.2lf, frame = %zu, x = %.2lf, y = %.2lf, z = %.2lf, q = [%.2lf, %.2lf, %.2lf, %.2lf]", 
        pose.timestamp, pose.frame, pose.x, pose.y, pose.z, pose.q[0], pose.q[1], pose.q[2], pose.q[3]);
    pose = darrlast(poses);
    DBG("last entry: timestamp = %.2lf, frame = %zu, x = %.2lf, y = %.2lf, z = %.2lf, q = [%.2lf, %.2lf, %.2lf, %.2lf]", 
        pose.timestamp, pose.frame, pose.x, pose.y, pose.z, pose.q[0], pose.q[1], pose.q[2], pose.q[3]);
    // free
    darrfree(poses);
    darrfree(contents);
    return 0;
}
