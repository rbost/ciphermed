#pragma once

struct Key_dependencies_descriptor {
    bool need_server_gm;
    bool need_server_paillier;
    bool need_server_fhe;

    bool need_client_gm;
    bool need_client_paillier;
    bool need_client_fhe;

    Key_dependencies_descriptor(bool server_gm, bool server_paillier, bool server_fhe, bool client_gm, bool client_paillier, bool client_fhe)
    : need_server_gm(server_gm), need_server_paillier(server_paillier),  need_server_fhe(server_fhe),  need_client_gm(client_gm),  need_client_paillier(client_paillier),  need_client_fhe(client_fhe)
    {};
};