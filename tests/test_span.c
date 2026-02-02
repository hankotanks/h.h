#define HH_IMPLEMENTATION
#define HH_STRIP_PREFIXES
#include "h.h"

#include <stdbool.h>

struct pose {
    double stamp;
    size_t frame;
    double xyz[3], q[4];
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
    // initialize parser
    span_t parser = span(contents);
    // count the number of columns in the header
    span_t header = span_next(&parser, .delim = "\n", .trim = true);
    size_t header_count = 0;
    while(span_next(&header, .delim = ",", .eol = true).ptr) ++header_count;
    ASSERT(header_count == 9, "Counted incorrect number of columns in header");
    // loop through lines and parse poses
    struct pose* poses = NULL;
    while(span_len(parser) > 0) {
        span_t err = {0};
        // parse the next pose
        struct pose curr = {
            .stamp = span_next_lf(&parser, &err, .delim = ",", .trim = true),
            .frame = span_next_zu(&parser, &err, .delim = ",", .trim = true),
            .xyz = {  
                span_next_lf(&parser, &err, .delim = ",", .trim = true),
                span_next_lf(&parser, &err, .delim = ",", .trim = true),
                span_next_lf(&parser, &err, .delim = ",", .trim = true) },
            .q = {
                span_next_lf(&parser, &err, .delim = ",", .trim = true),
                span_next_lf(&parser, &err, .delim = ",", .trim = true),
                span_next_lf(&parser, &err, .delim = ",", .trim = true),
                span_next_lf(&parser, &err, .delim = ",", .trim = true, .eol = true) }
        };
        // check if it failed
        ASSERT(err.ptr == NULL, "Failed to parse on token: " span_fmt, span_fmt_args(err));
        // add entry to pose array on success
        darrput(poses, curr);
    }
    ASSERT(darrlen(poses) == 689, "Failed to read correct number of lines");
    // free original content
    darrfree(contents);
    // calculate average position
    double pose_avg[3] = {0};
    for(size_t i = 0; i < darrlen(poses); ++i) {
        pose_avg[0] += poses[i].xyz[0];
        pose_avg[1] += poses[i].xyz[1];
        pose_avg[2] += poses[i].xyz[2];
    }
    pose_avg[0] /= (double) darrlen(poses);
    pose_avg[1] /= (double) darrlen(poses);
    pose_avg[2] /= (double) darrlen(poses);
    DBG("avg. pos. of %zu odom. entries: xyz = [%.2lf, %.2lf, %.2lf]", darrlen(poses), pose_avg[0], pose_avg[1], pose_avg[2]);
    // print first and last pose
    struct pose pose = poses[0];
    DBG("first entry: timestamp = %.2lf, frame = %zu, xyz = [%.2lf, %.2lf, %.2lf], q = [%.2lf, %.2lf, %.2lf, %.2lf]", 
        pose.stamp, pose.frame, pose.xyz[0], pose.xyz[1], pose.xyz[1], pose.q[0], pose.q[1], pose.q[2], pose.q[3]);
    pose = darrlast(poses);
    DBG("last entry: timestamp = %.2lf, frame = %zu, xyz = [%.2lf, %.2lf, %.2lf], q = [%.2lf, %.2lf, %.2lf, %.2lf]", 
        pose.stamp, pose.frame, pose.xyz[0], pose.xyz[1], pose.xyz[1], pose.q[0], pose.q[1], pose.q[2], pose.q[3]);
    // free the pose array
    darrfree(poses);
    return 0;
}
