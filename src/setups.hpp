#include <Wingine.hpp>

namespace wg {
  struct PipelineSetup {
    
    bool clear_color = true;
    bool clear_depth = true;

    void setClear(bool );
    void setClearColor();
    
  };
};
