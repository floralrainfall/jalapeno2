#ifndef BRAIN_HPP
#define BRAIN_HPP

#define MAX_NEURONS 24
#include <vector>
#include <imgui/imgui.h>

namespace TE
{
    enum NeuronInputNames
    {
        NIN_RS,
        NIN_RC,
        NIN_PX,
        NIN_PY,
        NIN_CR1,
        NIN_CR2,
        NIN_FPX,
        NIN_FPY,
        NIN_FOODLOOK,
        NIN_ODISTANCE,
        NIN_LCDISTANCE,
        NIN_GROWTH,
        NIN_HEALTH,
        NIN_ENERGY,
        NIN_STAMINA,
        NIN_PREGNANCY,
        NIN_M_MOVEFORWARD,
        NIN_M_MOVESIDEWAYS,
        NIN_M_ROTATE,
        NIN_M_MOUTHSIZE,
        NUM_INPUT_NEURONS,
    };
    const char* GetNINName(TE::NeuronInputNames name);

    enum NeuronOutputNames
    {
        NON_ROTATE,
        NON_MOVEFORWARD,
        NON_MOVESIDEWAYS,
        NON_MOUTHSIZE,
        NON_TOXINRANGE,
        NUM_OUTPUT_NEURONS,
    };

    enum NeuronType
    {
        MULTIPLY,
        MULTIPLY2,
        MEMORY_IN,
        MEMORY_OUT,
        SUBTRACT,
        DIVIDE,
        ADD,
        Count,
    };

    struct Neuron
    {
        float value_out;
        int output;
        NeuronType type;
        int input1;
        int input2;
        float value;
        float x;
        float y;
        int generation; // how old this neuron is
    };
    
    class Brain
    {
    private:
        float memory[201];
        void Mutate_AddNeuron();
        void Mutate_KillNeuron();
        void Mutate_ChangeNeuron();
        float EvaluateNeuron(int i);
        int NeuronsPointingTo(int output);
        int GetColorForNType(NeuronType type);
    public:
        static float average_neurons;
        static int total_neuron_count;
        static int total_brain_count;
        bool write_memories = true;

        Brain();
        ~Brain();

        float inputs[NUM_INPUT_NEURONS];
        float outputs[NUM_OUTPUT_NEURONS];

        std::vector<Neuron>* neurons;

        void Evaluate(bool o = false);
        void Mutate();
        void Copy(Brain* to);
        void DrawWidgets(ImDrawList* list);

    };
};

#endif