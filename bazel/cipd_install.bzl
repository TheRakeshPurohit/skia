"""This module defines the cipd_install repository rule.

The cipd_install rule is a wrapper around http_archive to download the CIPD
package at the specified version over HTTPS. This does not require depot_tools nor a cipd binary
on the host machine.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def cipd_download_urls(cipd_package, sha256, tag):
    """Construct download URLs for the given CIPD package.

    Args:
        cipd_package: The full name of the CIPD package. This is a "path" from the root of CIPD.
            This should be a publicly accessibly package, as authentication is not supported.
        sha256: The sha256 hash of the zip archive downloaded from CIPD. This should match the
            official CIPD website.
        tag: Represnts the version of the CIPD package to download, e.g. "git_package:abc123...".
    Returns:
        A string list containing download URLs for the given CIPD package.
    """
    cipd_url = "https://chrome-infra-packages.appspot.com/dl/"
    cipd_url += cipd_package
    cipd_url += "/+/"

    # When Bazel downloads the CIPD package, the local file name is "<tag>.zip". On Windows, colon
    # is not a valid character for file names, so this causes Bazel to crash. URL-escaping the tag
    # results in a valid file name.
    cipd_url += tag.replace(":", "%3A")

    mirror_url = "https://cdn.skia.org/bazel/"
    mirror_url += sha256
    mirror_url += ".zip"

    return [cipd_url, mirror_url]

def cipd_install(
        name,
        cipd_package,
        sha256,
        tag,
        build_file = None,
        build_file_content = None,
        postinstall_cmds_posix = None,
        postinstall_cmds_win = None):
    """Download and extract the zipped archive from CIPD, making it available for Bazel rules.

    Args:
        name: The name of the Bazel "repository" created. For example, if name is "alpha_beta",
            the full Bazel label will start with "@alpha_beta//".
        cipd_package: The full name of the CIPD package. This is a "path" from the root of CIPD.
            This should be a publicly accessible package, as authentication is not
            supported.
        sha256: The sha256 hash of the zip archive downloaded from CIPD. This should match the
            official CIPD website.
        tag: Represents the version of the CIPD package to download, e.g. "git_package:abc123...".
        build_file: The file to use as the BUILD.bazel file for this repository. Such build files
            typically contain "exports_files" and/or "filegroup" rules. Since CIPD packages do not
            include BUILD.bazel files, we must provide our own. Either build_file or
            build_file_content can be specified, but not both.
        build_file_content: The content for the BUILD file for this repository. Either build_file
            or build_file_content can be specified, but not both.
        postinstall_cmds_posix: Optional Bash commands to run on Mac/Linux after download.
        postinstall_cmds_win: Optional Powershell commands to run on Windows after download.
    """
    urls = cipd_download_urls(cipd_package, sha256, tag)

    http_archive(
        name = name,
        sha256 = sha256,
        urls = urls,
        type = "zip",
        build_file = build_file,
        build_file_content = build_file_content,
        patch_cmds = postinstall_cmds_posix,
        patch_cmds_win = postinstall_cmds_win,
    )
