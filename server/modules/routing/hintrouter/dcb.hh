#pragma once
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2020-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#include "hintrouterdefs.hh"

#include <tr1/memory>

#include <maxscale/dcb.h>

class Dcb
{
public:
    typedef std::tr1::shared_ptr<DCB> SDCB;

    explicit Dcb(DCB* pDcb);

    Dcb(const Dcb& rhs)
        : m_sInner(rhs.m_sInner)
    {};

    Dcb& operator = (Dcb rhs)
    {
        m_sInner.swap(rhs.m_sInner);
        return *this;
    }

    struct server* server() const
    {
        return (this->m_sInner.get()) ? m_sInner.get()->server : NULL;
    }

    DCB* get() const
    {
        return m_sInner.get();
    }

    bool write(GWBUF* pPacket) const
    {
        ss_dassert(m_sInner.get());
        return m_sInner.get()->func.write(m_sInner.get(), pPacket) == 1;
    }

private:
    static void deleter(DCB* dcb);
    SDCB m_sInner;
};

