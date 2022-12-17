#include "brain.hpp"
#include <random>
#include <imgui/imgui.h>
#include "organism.hpp"

static std::random_device rd;
static std::mt19937 gen(rd());

int TE::Brain::total_neuron_count = 0;
int TE::Brain::total_brain_count = 0;
float TE::Brain::average_neurons = 0;

const char* TE::GetNINName(TE::NeuronInputNames name)
{
    switch (name)
    {
    case NeuronInputNames::NIN_STAMINA:
        return "STAM";
    case NeuronInputNames::NIN_ENERGY:
        return "ENER";
    case NeuronInputNames::NIN_FOODLOOK:
        return "F+-?";
    case NeuronInputNames::NIN_FPX:
        return "FPdX";
    case NeuronInputNames::NIN_FPY:
        return "FPdY";
    case NeuronInputNames::NIN_CR1:
        return "CRC1";
    case NeuronInputNames::NIN_CR2:
        return "CRC2";
    case NeuronInputNames::NIN_HEALTH:
        return "HP";
    case NeuronInputNames::NIN_GROWTH:
        return "GROW";
    case NeuronInputNames::NIN_LCDISTANCE:
        return "CHIL";
    case NeuronInputNames::NIN_ODISTANCE:
        return "DIST";
    case NeuronInputNames::NIN_PREGNANCY:
        return "PREG";
    case NeuronInputNames::NIN_PX:
        return "OPpX";
    case NeuronInputNames::NIN_PY:
        return "OPpY";
    case NeuronInputNames::NIN_RC:
        return "RC";
    case NeuronInputNames::NIN_RS:
        return "RS";
    default:
        return "??";
    }
}

int TE::Brain::NeuronsPointingTo(int output)
{
    int nn = 0;
    for(int i = 0; i < neurons->size(); i++)
    {
        Neuron n = neurons->at(i);
        if(n.output == output)
            nn++;
    }
    return nn;
}

void TE::Brain::Evaluate(bool o)
{
    if(o)
    {
        total_neuron_count += neurons->size();
        average_neurons += (float)neurons->size();
        total_brain_count++;
    }
    inputs[NIN_M_ROTATE] = outputs[NON_ROTATE];
    inputs[NIN_M_MOVEFORWARD] = outputs[NON_MOVEFORWARD];
    inputs[NIN_M_MOVESIDEWAYS] = outputs[NON_MOVESIDEWAYS];
    inputs[NIN_M_MOUTHSIZE] = outputs[NON_MOUTHSIZE];
    for(int i = 0; i < sizeof(outputs)/sizeof(float); i++)
    {
        outputs[i] = 0.f;
    }
    for(int i = 0; i < neurons->size(); i++)
    {
        Neuron n = neurons->at(i);
        int nn = NeuronsPointingTo(n.output);
        outputs[n.output] += EvaluateNeuron(i);
    }
    for(int i = 0; i < sizeof(outputs)/sizeof(float); i++)
    {
        int nn = NeuronsPointingTo(i);
        outputs[i] /= nn?nn:1;
    }
}

float TE::Brain::EvaluateNeuron(int i)
{
    Neuron n = neurons->at(i);
    float intermediate;
    switch(n.type)
    {
        case NeuronType::ADD:
            intermediate = (inputs[n.input1] + inputs[n.input2]) * n.value;
            break;
        case NeuronType::SUBTRACT:
            intermediate = (inputs[n.input1] - inputs[n.input2]) * n.value;
            break;
        case NeuronType::DIVIDE:
            intermediate = (inputs[n.input1] / inputs[n.input2] ? inputs[n.input2] : 0) * n.value;
            break;
        case NeuronType::MULTIPLY:
            intermediate = inputs[n.input1] * n.value;
            break;
        default:
        case NeuronType::MULTIPLY2:
            intermediate = (inputs[n.input1] * inputs[n.input2]) * n.value;
            break;
    }
    CLIP(intermediate);
    return intermediate;
}

static std::uniform_int_distribution<> input_dist(0, TE::NUM_INPUT_NEURONS-1);
static std::uniform_int_distribution<> output_dist(0, TE::NUM_OUTPUT_NEURONS-1);
static std::uniform_int_distribution<> type_dist(0, TE::NeuronType::Count-1);
static std::uniform_real_distribution<> function_dist(-1, 1);
static std::uniform_real_distribution<> smallfunction_dist(-0.15, 0.15);
static std::uniform_real_distribution<> position_dist(460.f, 40.f);

void TE::Brain::Mutate_AddNeuron()
{
    if(neurons->size() >= MAX_NEURONS)
        return Mutate_KillNeuron();

    NeuronType ntype = (NeuronType)type_dist(gen);
    Neuron n;
    n.input1 = input_dist(gen);
    n.input2 = input_dist(gen);
    n.output = output_dist(gen);
    n.type = (NeuronType)type_dist(gen);
    n.value = function_dist(gen);
    n.x = position_dist(gen);
    n.y = position_dist(gen);
    n.generation = 0;
    neurons->push_back(n);
}

void TE::Brain::Mutate_ChangeNeuron()
{
    if(neurons->size() <= 1)
        return Mutate_AddNeuron();

    std::uniform_int_distribution<> neuron_dist(0, neurons->size()-1);
    Neuron n = neurons->at(neuron_dist(gen));

    if(n.generation > 5)
        n.value += smallfunction_dist(gen);
    else
        n.value += function_dist(gen);
    CLIP(n.value);
}

void TE::Brain::Mutate_KillNeuron()
{
    if(neurons->size() <= 1)
        return Mutate_AddNeuron();

    std::uniform_int_distribution<> elimination_dist(0, neurons->size()-1);
    int ns = elimination_dist(gen);
    Neuron n = neurons->at(ns);

    if(n.generation > 7 && n.generation < 14)
        return;

    neurons->erase(neurons->begin()+ns);
}

void TE::Brain::Mutate()
{
    float sel = function_dist(gen);
    float abs_sel = std::abs(sel);

    if(abs_sel < 0.7f)
    {
        if(sel < 0.f)
        {
            if(sel < 0.5f)
                Mutate_ChangeNeuron();
            Mutate_AddNeuron();
        }
        else if(sel > 0.f)
        {
            Mutate_KillNeuron();
        }
    }
}

void TE::Brain::Copy(TE::Brain* to)
{
    for(int i = 0; i < neurons->size(); i++)
    {
        Neuron n = neurons->at(i);
        n.generation++;
        to->neurons->push_back(n);
    }
}

TE::Brain::Brain()
{
    neurons = new std::vector<Neuron>();
}

TE::Brain::~Brain()
{
    delete neurons;
}

#define NEURON_SIZE 20.1f
#define OFFSET_X (wpos.x+50.f)
#define OFFSET_Y (wpos.y+80.f)
#define TEXT_OFFSET -16.f

int TE::Brain::GetColorForNType(NeuronType t)
{
    switch(t)
    {
        case NeuronType::ADD:
            return 0xff00ff55;
            break;
        case NeuronType::MEMORY_IN:
            return 0xff0000ff;
            break;
        case NeuronType::MEMORY_OUT:
            return 0xff0055ff;
            break;
        case NeuronType::MULTIPLY:
            return 0xffffffff;
            break;
        case NeuronType::MULTIPLY2:
            return 0xffdddddd;
            break;
        default:
            break;
    }
    return 0xffffffff;
}

void TE::Brain::DrawWidgets(ImDrawList* drawList)
{
    bool wm = write_memories;
    write_memories = false;
    ImVec2 wpos = ImGui::GetWindowPos();
    if(ImGui::BeginChild("Neuron Brain",ImVec2(sizeof(inputs)/sizeof(float)*20.1f,200.f)));
    {
        char floattxt[64];
        for(int i = 0; i < neurons->size(); i++)
        {
            Neuron n = neurons->at(i);
            drawList->AddLine(ImVec2(OFFSET_X+n.input1*(NEURON_SIZE*2.f), OFFSET_Y), ImVec2(OFFSET_X+n.x, OFFSET_Y+n.y), 0xffffffff);
            if(n.type != MULTIPLY)
            {
                drawList->AddLine(ImVec2(OFFSET_X+n.input2*(NEURON_SIZE*2.f), OFFSET_Y), ImVec2(OFFSET_X+n.x, OFFSET_Y+n.y), 0xffffffff);
            }
            drawList->AddLine(ImVec2(OFFSET_X+n.output*(NEURON_SIZE*2.f), OFFSET_Y+500.f), ImVec2(OFFSET_X+n.x, OFFSET_Y+n.y), 0xffffffff);
            drawList->AddCircleFilled(ImVec2(OFFSET_X+n.x, OFFSET_Y+n.y), (NEURON_SIZE), GetColorForNType(n.type));
            drawList->AddCircle(ImVec2(OFFSET_X+n.x, OFFSET_Y+n.y), (NEURON_SIZE), 0xffffffff);
            snprintf(floattxt, 64, "%.2f", n.value);
            drawList->AddText(ImVec2(OFFSET_X+n.x+TEXT_OFFSET, OFFSET_Y+n.y+TEXT_OFFSET), 0xff000000, floattxt);
            snprintf(floattxt, 64, "%.2f", EvaluateNeuron(i));
            drawList->AddText(ImVec2(OFFSET_X+n.x+TEXT_OFFSET, 16.f+OFFSET_Y+n.y+TEXT_OFFSET), 0xff000000, floattxt);
            snprintf(floattxt, 64, "%i", n.generation);
            drawList->AddText(ImVec2(OFFSET_X+n.x+TEXT_OFFSET, OFFSET_Y+n.y+TEXT_OFFSET-16.f), 0xff0000ff, floattxt);
            drawList->AddText(ImVec2(OFFSET_X+n.x+TEXT_OFFSET+NEURON_SIZE*2, OFFSET_Y+n.y+TEXT_OFFSET), 0xff0000ff, GetNINName((NeuronInputNames)n.input1));
            if(n.type != MULTIPLY)
                drawList->AddText(ImVec2(OFFSET_X+n.x+TEXT_OFFSET+NEURON_SIZE*2, OFFSET_Y+n.y), 0xff0000ff, GetNINName((NeuronInputNames)n.input2));
        }
        for(int i = 0; i < sizeof(inputs)/sizeof(float); i++)
        {
            drawList->AddCircleFilled(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f), OFFSET_Y), NEURON_SIZE, 0xff00ffff);
            snprintf(floattxt, 64, "%.2f", inputs[i]);
            drawList->AddText(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f)+TEXT_OFFSET, OFFSET_Y+TEXT_OFFSET), 0xff0000ff, floattxt);
            drawList->AddText(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f)+TEXT_OFFSET, OFFSET_Y+TEXT_OFFSET-16.f), 0xff0000ff, GetNINName((NeuronInputNames)i));
        }
        for(int i = 0; i < sizeof(outputs)/sizeof(float); i++)
        {
            drawList->AddCircleFilled(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f), OFFSET_Y+500.f), (NEURON_SIZE), 0xff00ffff);
            snprintf(floattxt, 64, "%.2f", outputs[i]);
            drawList->AddText(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f)+TEXT_OFFSET, OFFSET_Y+500.f+TEXT_OFFSET), 0xff0000ff, floattxt);
            snprintf(floattxt, 64, "%i", NeuronsPointingTo(i));
            drawList->AddText(ImVec2(OFFSET_X+i*(NEURON_SIZE*2.f)+TEXT_OFFSET, 16.f+OFFSET_Y+500.f+TEXT_OFFSET), 0xff0000ff, floattxt);
        }
    }
    ImGui::EndChild();
    write_memories = wm;
}