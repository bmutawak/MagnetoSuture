%% Load in data
clear; clc; close all;
nn_locs = load('particle_locs_NN.txt');
sf_locs = load('particle_locs_SF.txt');
desired_locs_comparison = load('desired_locs_comparison.txt');
desired_locs_average = load('desired_locs_average.txt');


%% Plot SF vs. NN
figure; hold on;
plot(desired_locs_comparison(:, 1), desired_locs_comparison(:, 2), '-k', 'LineWidth', 2.5, 'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r', 'MarkerSize', 12);
plot(nn_locs(:, 1), nn_locs(:, 2), '--b', 'LineWidth', 2, 'MarkerFaceColor', 'b', 'MarkerEdgeColor', 'b', 'MarkerSize', 8);
plot(sf_locs(:, 1), sf_locs(:, 2), '--g', 'LineWidth', 2, 'MarkerFaceColor', 'g', 'MarkerEdgeColor', 'g', 'MarkerSize', 8);
legend('Expected Path', 'Actual Path (NN)', 'Actual Path (SF)');
xlabel('X-Coordinates (mm)', 'FontSize', 13); ylabel('Y-Coordinates (mm)', 'FontSize', 13);
ax = gca;
xticks(-12:2:13);
ax.FontSize = 10; 
title('Actual Path Traversed By Particle vs. Desired Path', 'FontSize', 18);