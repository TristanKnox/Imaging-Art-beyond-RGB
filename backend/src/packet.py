""" target.py
dataclasses and functions related to the packet

Structs:
    Packet
    Target

Functions:
    load_calibration_imgs : Load images needed for calibration
    get_white_img         : Get tuple of flat field images
    get_dark_img          : Get tuple of dark field images
    get_target_img        : Get tuple of target images
    generate_swap         : Generate swap space
    unload_white          : Unload flat field images
    unload_dark           : Unload dark field images
    unload_target         : Unload target images

Authors:
    Brendan Grau <https://github.com/Victoriam7>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
import gc
import numpy as np
from dataclasses import dataclass

from rgbio import load_image, load_array, save_array, create_temp_file
from constants import IMGTYPE_TARGET, IMGTYPE_WHITE,\
        IMGTYPE_DARK, IMGTYPE_SUBJECT

""" Constants """
# File/array index constants
__WHITE_A_IDX = 0
__WHITE_B_IDX = 1
__DARK_A_IDX = 2
__DARK_B_IDX = 3
__TARGET_A_IDX = 4
__TARGET_B_IDX = 5
__RENDERABLES_START = 6


@dataclass
class Packet:
    """ Packet
    struct to hold pipeline data

    Members:
        files   : list of image files
        swap    : list of temp files for storing image arrays
        subjptr : tuple containing indices of current subject for batch
        target  : dataclass for the target
        wscale  : white patch scale value
        mcalib  : calibrated matrix
        dims    : TODO delete
        camsigs : TODO delete
        render  : TODO delete
    """
    files: list
    swap: list
    subjptr: tuple
    target: np.ndarray
    wscale: tuple
    mcalib: np.ndarray
    camsigs: np.ndarray
    render: np.ndarray


def packetgen(files: list) -> Packet:
    """ Initialize packet with default values
    [in] files: list of files we are working with
    [out] packet
    """
    pkt = Packet()
    pkt.files = files
    pkt.swap = __swapgen(len(files) // 2)
    pkt.subject_idx = (__TARGET_A_IDX, __TARGET_B_IDX)

    return pkt


def imgget(packet: Packet, imgtype: int) -> tuple:
    """ Get specified image
    Loads image from swap space and returns it
    [in] packet  : packet we are operating on
    [in] imgtype : image type as specified in constants
    [out] tuple containing specified (A, B) image pair
    """
    if imgtype == IMGTYPE_TARGET:
        a, b = __TARGET_A_IDX, __TARGET_B_IDX
    elif imgtype == IMGTYPE_WHITE:
        a, b = __WHITE_A_IDX, __WHITE_B_IDX
    elif imgtype == IMGTYPE_DARK:
        a, b = __DARK_A_IDX, __DARK_B_IDX
    elif imgtype == IMGTYPE_SUBJECT:
        a, b = packet.subjptr[0], packet.subjptr[1]
    else:
        return None, None

    swapidx = __swapgen(a)
    return __imgload(packet, a, b, swapidx)


def imgput(packet: Packet, imgtype: int, imgpair: tuple) -> None:
    """ Put image back in swap
    only call if image contents need to be saved otherwise just delete
    [in] packet  : packet we are operating on
    [in] imgtype : image type as specified in constants
    [in] imgpair : image pair to be saved
    """
    if imgtype == IMGTYPE_TARGET:
        s = __TARGET_A_IDX // 2
    elif imgtype == IMGTYPE_WHITE:
        s = __WHITE_A_IDX // 2
    elif imgtype == IMGTYPE_DARK:
        s = __DARK_A_IDX // 2
    elif imgtype == IMGTYPE_SUBJECT:
        s = packet.subjptr[0] // 2
    else:
        return

    save_array((imgpair[0], imgpair[1]), packet.swap[s])

    del imgpair[0], imgpair[1]
    gc.collect()


def __swapload(packet: Packet) -> None:
    """ Populate swap files with their corresponding images
    [in] packet : packet we are operating on
    [post] temp files have been loaded with the images
    """
    for i, s in enumerate(packet.swap):
        # Gen a,b indices into file list
        a = i * 2
        b = a + 1
        # Load files
        aimg, bimg = __imgload(packet, a, b)


def __swapidxgen(idx: int) -> int:
    """ Get index of corresponding swap file
    [in] idx : index of image in file list
    [out] index of image is swap list
    """
    return idx // 2


def __swapgen(n: int):
    """ Generate swapfiles
    [in] n : number of files to generate
    [out] swap list
    """
    swap = []
    for i in range(0, n):
        swap.append(create_temp_file())
    return swap


def __imgload(packet: Packet, a: int, b: int, s: int = None) -> tuple:
    """ Load image pair
    [in] packet : packet we are operating on
    [in] a      : A file index
    [in] b      : B file index
    [in, opt] s : swap file location index
    [out] image pair in (A, B) tuple
    """
    if s:
        return load_array(packet.swap[s])
    else:
        aimg = load_image(packet.files[a])
        bimg = load_image(packet.files[b])
        return aimg, bimg
