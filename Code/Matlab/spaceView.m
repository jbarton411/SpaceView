figure(1);
plot(Time, Altitude);
title('Altitude vs. Time');
xlabel('Time');
ylabel('Altitude');

figure(2);
plot(Altitude, Temperature);
xlabel('Altitude');
ylabel('Temperature');

figure(3);
plot(Latitude, Longitude);
xlabel('Latitude');
ylabel('Longitude');

figure(4);
plot(Time, Pressure);
xlabel('Time');
ylabel('Pressure');

figure(5);
plot(Pressure, Temperature);
xlabel('Pressure');
ylabel('Temperature');

%figure(6);
%plot(Altitude, Pressure);
%xlabel('Altitude');
%ylabel('Pressure');

% figure(7);
% plot(Altitude, Acceleration);
% xlabel('Altitude');
% ylabel('Acceleration');

figure(8);
plot(Pressure, Accelerationz);
xlabel('Pressure');
ylabel('Acceleration');

figure(9);
plot(Time, Temperature);
xlabel('Time');
ylabel('Temperature');

Velocity_z = zeros(length(Accelerationz), 1);
Velocity_z(n,1) = Accelerationz(n,1) * Time(n,1);

for n = 2:length(Velocity_z);
    Velocity_z(n,1) = Velocity_z[n-1,1] + (Accelerationz[n,1] * (Time[n,1]-Time[n-1,1]));
end
