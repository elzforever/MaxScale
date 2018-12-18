/*
 * Copyright (c) 2018 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2022-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#include "clustrixmonitor.hh"

namespace http = mxb::http;
using namespace std;

ClustrixMonitor::ClustrixMonitor(MXS_MONITOR* pMonitor)
    : maxscale::MonitorInstance(pMonitor)
{
}

//static
ClustrixMonitor* ClustrixMonitor::create(MXS_MONITOR* pMonitor)
{
    return new ClustrixMonitor(pMonitor);
}

bool ClustrixMonitor::configure(const MXS_CONFIG_PARAMETER* pParams)
{
    m_health_urls.clear();

    MXS_MONITORED_SERVER* pMonitored_server = m_monitor->monitored_servers;

    while (pMonitored_server)
    {
        SERVER* pServer = pMonitored_server->server;

        string url(pServer->address);
        url += ":";
        url += "3581"; // TODO: Make configurable.

        pMonitored_server = pMonitored_server->next;
    }

    return true;
}

void ClustrixMonitor::tick()
{
    MXS_NOTICE("ClustrixMonitor::tick()");

    vector<http::Result> results = http::get(m_health_urls);

    for (size_t i = 0; i < m_health_urls.size(); ++i)
    {
        const auto& url = m_health_urls[i];
        const auto& result = results[i];

        MXS_NOTICE("%s: %s", url.c_str(), (result.code == 200) ? "OK" : result.body.c_str());
    }
}
