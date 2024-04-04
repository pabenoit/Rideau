#ifndef NTP_H
#define NTP_H

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

#endif