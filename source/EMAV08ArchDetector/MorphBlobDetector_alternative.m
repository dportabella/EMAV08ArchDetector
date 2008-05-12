function X=blob_detector_morph(srcImage)
% The aim of the blob detector is to find the balloons and plates of the arch.
% This is not the version implemented using the real-time c++ implementation.
% This matlab version provides better results than the c++ version,
% however, it stills needs to be redesigned, as it does not produce good results
% with the new videos taken.
%
%
% EMAV08ArchDetector is a computer vision software to detect the arches
% from a video-stream for the EMAV08 competition.
%
%  The rules of the EMAV08 competition are given in
%  http://www.dgon.de/content/pdf/emav_2008_Rules_v07.pdf
%  (attached also in this zip file)
%  See also the EMAV08 website for more details: http://www.dgon.de/emav2008.htm
%
% EMAV08ArchDetector
% Copyright (C) 2008 David Portabella Clotet
%
% To contact the author:
% email: david.portabella@gmail.com
% web: http://david.portabella.name
% 
%
% This file is part of EMAV08ArchDetector
%  
% EMAV08ArchDetector is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
%
% EMAV08ArchDetector is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with EMAV08ArchDetector.  If not, see <http://www.gnu.org/licenses/>.
%

  I=rgb2gray(srcImage);
  edges = edge(I,'canny');

  %%bw = bwareaopen(bw,30);
  %se = strel('disk',2);
  %bw = imclose(bw,se);
  %figure; imshow(bw);

  bw = imfill(edges,'holes');
  %figure; imshow(bw);

  bw = my_erode(bw);

  %figure; imshow(imerode(bw,strel('square',2)));
  %figure; imshow(lighterZone(srcImage, double(imerode(bw,strel('square',2))), [255 255 255], true));
  %figure; imshow(lighterZone(srcImage, double(erode_test(bw)), [255 255 255], true));

  [B,L] = bwboundaries(bw,'noholes');

  %f=figure;
  %%Display the label matrix and draw each boundary
  %imshow(label2rgb(L, @jet, [.5 .5 .5]))
  %hold on
  %for k = 1:length(B)
  %  boundary = B{k};
  %  %plot(boundary(:,2), boundary(:,1), 'b', 'LineWidth', 2)
  %end


  imshow(srcImage);
  hold on; 
  
  %threshold = 0.94;
  threshold = 0.7;
  X=zeros(2,0);
  i = 1;
  stats = regionprops(L,'Area','Centroid');

  % loop over the boundaries
  for k = 1:length(B)
    % obtain (X,Y) boundary coordinates corresponding to label 'k'
    boundary = B{k};

    % compute a simple estimate of the object's perimeter
    delta_sq = diff(boundary).^2;    
    perimeter = sum(sqrt(sum(delta_sq,2)));

    % obtain the area calculation corresponding to label 'k'
    area = stats(k).Area;

    % compute the roundness metric
    metric = 4*pi*area/perimeter^2;

    %fprintf('%d pos=%f,%f  area=%f perim=%5.3f metric=%5.3f\n', k, stats(k).Centroid(1), stats(k).Centroid(2), area, perimeter, metric);

    if metric > threshold
      X(1,i) = stats(k).Centroid(1);
      X(2,i) = stats(k).Centroid(2);
      i = i+1;

      plot(boundary(:,2), boundary(:,1), 'b');
      centroid = stats(k).Centroid;
      plot(centroid(1),centroid(2),'+g');
      %text(boundary(1,2)-35,boundary(1,1)+13,metric_string,'Color','y','FontSize',14,'FontWeight','bold');
    else
      L = L .* (L~=k);
    end

    %f=figure; imshow(L==k); pause; close(f);
  end

  %bw = L > 0;
  %outImg = lighterZone(srcImage, double(bw), [255 255 255], true);
  %imshow(outImg); 
return




function bw = my_erode(bw)
  temp = zeros(size(bw,1)+2, size(bw,2)+2);
  temp(2:size(bw,1)+1, 2:size(bw,2)+1) = bw;

  changed = true;
  while changed == true
    changed = false;
    for y=2:size(temp,1)-1
      for x=2:size(temp,2)-1
        if (temp(y,x) == 1 && con(temp,y,x) < 3)
          temp(y,x) = 0;
          changed = true;
	    end
      end
    end
  end
  bw = temp(2:size(temp,1)-1, 2:size(temp,2)-1);
return

function v = con(bw,y,x)
  v = sum(sum(bw(y-1:y+1,x-1:x+1))) - 1;
return

