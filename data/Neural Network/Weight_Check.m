%% Load file you wish to find the weights
close all; clear all;
load('Minus_Y_Target_8_weights_regression.mat')
%% Plot performance plot: 
figure();
%plot performance by hidden layers 
plot((10:10:100),perf);
%The one with the lowest value will give you the best R value
% Use the weights from this Layer
%% Plot regression models: 
for i = 1:10
    figure();
    trOut = All_output{i}(tr{1,i}.trainInd);
    trTarg = All_Y_minus_target_data(tr{1,i}.trainInd);
    vOut = All_output{i}(tr{1,i}.valInd);
    vTarg = All_Y_minus_target_data(tr{1,i}.valInd);
    tsOut = All_output{i}(tr{1,i}.testInd);
    tsTarg = All_Y_minus_target_data(tr{1,i}.testInd);
    plotregression(trOut,trTarg,'train',vTarg, vOut, 'Validation', tsTarg, tsOut, 'Testing',All_output{i},All_Y_minus_target_data,'All')
end
%% choose weights from the IW with the lowest performance
[Best, Best_Loc] = min(perf);
Best_Weight = IW{1,Best_Loc}{1,1};