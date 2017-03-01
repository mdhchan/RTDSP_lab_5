clear all;
close all;

for order=6:6
[b,a] = ellip(order,0.3,20,[200 450]/4000);

figure;
freqz(b,a);
title('Frequency and Phase Response for Lab 5 IIR Bandpass Filter');


figure;
H = tf(b,a);
pzmap(H);
title('Pole-Zero Plot for Lab 5 IIR Bandpass Filter');
grid on;

%IIR coefficients output printed according to C-syntax
str = sprintf('IIR_coeff_DF2T_%d.txt',order*2);
fid = fopen(str,'w');
%fprintf(fid, '#define N %d\n', length(a));
%fprintf(fid, '#define M %d\n', length(b));
fprintf(fid, '// IIR filter coefficients \n');
fprintf(fid, 'double a[]={');
fprintf(fid, '%2.16e,\t', a);
fprintf(fid, '};\n');
fprintf(fid, 'double b[]={');
fprintf(fid, '%2.16e,\t', b);
fprintf(fid, '};\n');
fclose(fid);
end
