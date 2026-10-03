from __future__ import annotations

import argparse

from game import HelloGame


def requested_frames(arguments: argparse.Namespace) -> int | None:
    selected = sum((arguments.smoke_test, arguments.stability_test, arguments.frames is not None))
    if selected > 1:
        raise SystemExit("choose only one of --smoke-test, --stability-test, or --frames")
    if arguments.smoke_test:
        return 60
    if arguments.stability_test:
        return 600
    if arguments.frames is not None and arguments.frames <= 0:
        raise SystemExit("--frames must be positive")
    return arguments.frames


def main() -> None:
    parser = argparse.ArgumentParser(description="Run the real CNA-Python desktop starter")
    parser.add_argument("--smoke-test", action="store_true", help="draw exactly 60 real frames")
    parser.add_argument("--stability-test", action="store_true", help="draw exactly 600 real frames")
    parser.add_argument("--frames", type=int, help="draw exactly N real frames")
    parser.add_argument("--verify-frame", action="store_true",
                        help="also read the back buffer and check something was drawn")
    parser.add_argument("--verify-cnb", action="store_true",
                        help="compile and decode one .cnj asset through "
                             "cna.extensions.content, then exit without starting the game")
    parser.add_argument("--verify-engine", action="store_true",
                        help="check CNA's graphics extension layer through "
                             "cna.extensions.engine, then exit without starting the game")
    arguments = parser.parse_args()
    if arguments.verify_engine:
        # Separate for the same reason --verify-cnb is: the extension layer is a
        # CNA-only vocabulary, a game drawing sprites never touches it, and a CNA
        # build may be configured without it at all.
        from tools.verify_engine import run as verify_engine

        verify_engine()
        return
    if arguments.verify_cnb:
        # A separate check on purpose: `.cnb` is CNA's compiled content format,
        # and a game that ships `.xnb` or loose files never touches it. The
        # 60- and 600-frame runs must not need it.
        from tools.verify_cnb import run as verify_cnb

        verify_cnb()
        return
    frames = requested_frames(arguments)
    game = HelloGame(frames, verify_frame=arguments.verify_frame)
    with game:
        game.Run()
    if frames is not None:
        if game.DrawnFrames != frames:
            raise RuntimeError(f"requested {frames} frames but drew {game.DrawnFrames}")
        print(f"cna-python-template: SUCCESS drew {game.DrawnFrames} real CNA frames")
    if arguments.verify_frame:
        if not getattr(game, "FrameVerificationAvailable", False):
            print("cna-python-template: FRAME_VERIFICATION=unavailable "
                  "(this backend has no back-buffer pixel storage)")
        elif game.FrameVerified:
            print("cna-python-template: FRAME_VERIFICATION=drawn")
        else:
            raise RuntimeError("the frame contained nothing but the clear colour")


if __name__ == "__main__":
    main()
