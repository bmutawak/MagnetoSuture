from typing import Literal

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.axes import Axes
from scipy.io import loadmat
from scipy.stats import linregress


# -----------------------------------------------------------------------------
def create_nn_regression_plot(
    plus_coil_model_path: str,
    minus_coil_model_path: str,
    coil_axis: Literal["X", "Y"],
    show: bool = True,
) -> None:
    # load models data
    plus_coil_model_data: dict[str, np.ndarray] = loadmat(file_name=plus_coil_model_path)
    minus_coil_model_data: dict[str, np.ndarray] = loadmat(file_name=minus_coil_model_path)

    # retrieve the test targets (expected) and outputs (predicted)

    # NOTE: paper mentions the test data was used for figure 8a but this may
    # require some checking
    plus_coil_testInd: np.ndarray = plus_coil_model_data["tr"][0, 0]["testInd"][0][0].flatten() - 1
    minus_coil_testInd: np.ndarray = (
        minus_coil_model_data["tr"][0, 0]["testInd"][0][0].flatten() - 1
    )

    plus_coil_tsOut: np.ndarray = plus_coil_model_data["All_output"][0][0][0][plus_coil_testInd]
    plus_coil_tsTarg: np.ndarray = plus_coil_model_data["All_X_plus_target_data"][
        plus_coil_testInd
    ].T.flatten()

    minus_coil_tsOut: np.ndarray = minus_coil_model_data["All_output"][0][0][0][minus_coil_testInd]
    minus_coil_tsTarg: np.ndarray = minus_coil_model_data["All_X_minus_target_data"][
        minus_coil_testInd
    ].T.flatten()

    # calculate r2 value for plus coil and minus coil model's performance
    plus_coil_model_ts_r2: float = linregress(x=plus_coil_tsTarg, y=plus_coil_tsOut).rvalue ** 2
    minus_coil_model_ts_r2: float = linregress(x=minus_coil_tsTarg, y=minus_coil_tsOut).rvalue ** 2

    # plot figure 8a
    plt.figure(figsize=(6.8, 4.8))
    ax: Axes = plt.gca()
    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)

    plt.scatter(
        x=plus_coil_tsTarg,
        y=plus_coil_tsOut,
        s=8,
        label=f"+{coil_axis}: $R^{2}$ = {plus_coil_model_ts_r2:.5f}",
    )
    plt.scatter(
        x=minus_coil_tsTarg,
        y=minus_coil_tsOut,
        s=8,
        label=f"-{coil_axis}: $R^{2}$ = {minus_coil_model_ts_r2:.5f}",
    )
    plt.plot(
        np.linspace(0, plus_coil_tsTarg, num=len(plus_coil_tsTarg), endpoint=True).flatten(),
        np.linspace(0, plus_coil_tsTarg, num=len(plus_coil_tsTarg), endpoint=True).flatten(),
        color="lime",
        linewidth=3,
        label="Predicted = Expected",
    )

    plt.xlim(0, 90)
    plt.ylim(0, 90)
    plt.xticks(list(range(0, 91, 10)))
    plt.yticks(list(range(0, 91, 10)))
    plt.title(
        f"Predicted vs. Expected Current Scale Factor for +{coil_axis} and -{coil_axis} Coil",
        fontweight="bold",
    )
    plt.legend(loc="lower right")
    plt.savefig(
        "./figure8a-plot.png",
        dpi=400,
        bbox_inches="tight",
    )

    if show:
        plt.show()


# -----------------------------------------------------------------------------
def create_traversal_plots(
    expected_traversal_path: str,
    actual_nn_traversal_path: str,
    actual_sf_traversal_path: str,
    average_nn_traversal_path: str,
    show: bool = True,
) -> None:
    # load path points
    expected_traversal = pd.read_csv(expected_traversal_path, header="infer")
    actual_nn_traversal = pd.read_csv(actual_nn_traversal_path, header="infer")
    actual_sf_traversal = pd.read_csv(actual_sf_traversal_path, header="infer")
    average_nn_traversal = pd.read_csv(average_nn_traversal_path, header="infer")

    # create figure 10a
    plt.figure(figsize=(6.8, 4.8))
    ax: Axes = plt.gca()
    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)

    plt.plot(
        expected_traversal["x"],
        expected_traversal["y"],
        "k-",
        linewidth=2,
        label="Expected Path",
    )

    plt.plot(
        actual_nn_traversal["x"],
        actual_nn_traversal["y"],
        "b--",
        linewidth=2,
        label="Actual Path (NN)",
    )

    plt.plot(
        actual_sf_traversal["x"],
        actual_sf_traversal["y"],
        "--",
        color="lime",
        linewidth=2,
        label="Actual Path (SF)",
    )

    plt.xlim(-15, 15)
    plt.ylim(-15, 10)
    plt.xlabel("X-Coordinates (mm)")
    plt.ylabel("Y-Coordinates (mm)")
    plt.xticks(list(range(-12, 13, 2)))
    plt.title("Actual Path Traversed By Particle vs. Desired Path", fontweight="bold")
    plt.legend()
    plt.savefig(
        "./figure10a-plot.png",
        dpi=400,
        bbox_inches="tight",
    )

    if show:
        plt.show()

    # create figure 10b
    plt.figure(figsize=(6.8, 4.8))
    ax: Axes = plt.gca()
    ax.spines["right"].set_visible(False)
    ax.spines["top"].set_visible(False)

    plt.plot(
        expected_traversal["x"],
        expected_traversal["y"],
        "k-",
        linewidth=2,
        label="Expected Path",
    )

    plt.plot(
        average_nn_traversal["x"],
        average_nn_traversal["y"],
        "r--",
        linewidth=2,
        label="Mean Actual Path",
    )

    plt.xlim(-15, 15)
    plt.ylim(-15, 10)
    plt.xlabel("X-Coordinates (mm)")
    plt.ylabel("Y-Coordinates (mm)")
    plt.xticks(list(range(-12, 13, 2)))
    plt.title(
        "Neural Network - Expected vs. Average Translation for N = 10 Trials", fontweight="bold"
    )
    plt.legend()
    plt.savefig(
        "./figure10b-plot.png",
        dpi=400,
        bbox_inches="tight",
    )

    if show:
        plt.show()


# -----------------------------------------------------------------------------
if __name__ == "__main__":
    # NOTE: unsure if these are the right model files for the figure 8a data
    create_nn_regression_plot(
        plus_coil_model_path=r"./Neural Network/Plus_X_Target_5_weights_regression.mat",
        minus_coil_model_path=r"./Neural Network/Minus_X_Target_5_weights_regression.mat",
        coil_axis="X",
        show=True,
    )

    create_traversal_plots(
        expected_traversal_path=r"./plotdigitizer-data/expected-path-plot-data.csv",
        actual_nn_traversal_path=r"./plotdigitizer-data/actual-path-nn-plot-data.csv",
        actual_sf_traversal_path=r"./plotdigitizer-data/actual-path-sf-plot-data.csv",
        average_nn_traversal_path=r"./plotdigitizer-data/average-path-nn-plot-data.csv",
        show=True,
    )
