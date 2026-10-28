#include "phi.h"

#include "format.h"
#include "searchtree.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace noc
{

const char*
MethodName(Method m)
{
    return m == Method::Path ? "path" : "hop";
}

Method
ParseMethod(const std::string& name)
{
    for (Method m : kAllMethods)
    {
        if (name == MethodName(m))
        {
            return m;
        }
    }
    throw std::invalid_argument("method không hợp lệ: '" + name + "' (both | path | hop)");
}

ChannelPressure
ComputePressure(Algo algo, const Mesh& mesh, const std::vector<Flow>& flows)
{
    const int nch = mesh.NumChannels();
    ChannelPressure r{std::vector<double>(nch, 0.0), std::vector<double>(nch, 0.0)};
    std::vector<long> count(nch);  // số đường của cặp hiện tại đi qua kênh
    std::vector<double> hop(nch);  // Σ hopProb của các đường đó
    for (const Flow& f : flows)
    {
        std::fill(count.begin(), count.end(), 0);
        std::fill(hop.begin(), hop.end(), 0.0);
        long paths = 0;
        ForEachPath(algo, mesh, f.src, f.dst, [&](const std::vector<int>& nodes, double prob) {
            ++paths;
            for (std::size_t i = 0; i + 1 < nodes.size(); ++i)
            {
                const int ch = mesh.ChannelBetween(nodes[i], nodes[i + 1]);
                ++count[ch];
                hop[ch] += prob;
            }
        });
        for (int ch = 0; ch < nch; ++ch)
        {
            if (count[ch] > 0)
            {
                r.path[ch] += f.weight * static_cast<double>(count[ch]) / paths;
                r.hop[ch] += f.weight * hop[ch];
            }
        }
    }
    return r;
}

PhiResult
MaxPressure(const std::vector<double>& rho)
{
    PhiResult r{0.0, {}};
    for (double v : rho)
    {
        r.phi = std::max(r.phi, v);
    }
    for (std::size_t ch = 0; ch < rho.size(); ++ch)
    {
        if (rho[ch] >= r.phi - kTieEps)
        {
            r.maxChannels.push_back(static_cast<int>(ch));
        }
    }
    return r;
}

double
PirMax(double phi)
{
    return phi > 0.0 ? 0.0625 / phi : std::numeric_limits<double>::infinity();
}

namespace
{

std::string
JoinChannels(const Mesh& mesh, const std::vector<int>& chans)
{
    std::string s;
    for (int ch : chans)
    {
        s += (s.empty() ? "" : ";") + mesh.ChannelName(ch);
    }
    return s;
}

} // namespace

void
RunPhiMode(const PhiOptions& opt, std::ostream& os)
{
    const Mesh mesh(opt.w, opt.h);
    std::vector<Algo> algos;
    if (opt.routing == "all")
    {
        algos.assign(kAllAlgos.begin(), kAllAlgos.end());
    }
    else
    {
        algos.push_back(ParseAlgo(opt.routing));
    }
    std::vector<Pattern> patterns;
    if (opt.traffic == "all")
    {
        patterns.assign(kAllPatterns.begin(), kAllPatterns.end());
    }
    else
    {
        patterns.push_back(ParsePattern(opt.traffic));
    }
    std::vector<Method> methods;
    if (opt.method == "both")
    {
        methods.assign(kAllMethods.begin(), kAllMethods.end());
    }
    else
    {
        methods.push_back(ParseMethod(opt.method));
    }
    const std::string meshTag = std::to_string(mesh.W()) + "x" + std::to_string(mesh.H());

    std::ostringstream table;
    std::ostringstream results;
    table << "# " << std::left << std::setw(8) << "routing" << std::setw(11) << "traffic"
          << std::setw(5) << "meth" << std::right << std::setw(11) << "phi" << std::setw(11)
          << "phi_norm" << std::setw(11) << "PIR_max" << std::setw(6) << "nmax"
          << "  bottleneck\n";
    for (Algo algo : algos)
    {
        for (Pattern pattern : patterns)
        {
            if (pattern != Pattern::Uniform && mesh.W() != mesh.H() && opt.traffic == "all")
            {
                table << "# bỏ qua " << PatternName(pattern) << ": cần W == H\n";
                continue;
            }
            const std::vector<Flow> flows = MakeFlows(pattern, mesh);
            const ChannelPressure cp = ComputePressure(algo, mesh, flows);
            for (Method method : methods)
            {
                const std::vector<double>& rho = cp.Get(method);
                const PhiResult r = MaxPressure(rho);
                const double phiNorm = r.phi * NormFactor(pattern, mesh);
                const double pir = PirMax(phiNorm);
                const std::string maxch = JoinChannels(mesh, r.maxChannels);
                const std::string variant = VariantName(algo);

                table << "# " << std::left << std::setw(8) << AlgoName(algo) << std::setw(11)
                      << PatternName(pattern) << std::setw(5) << MethodName(method)
                      << std::right << std::setw(11) << Fixed(r.phi) << std::setw(11)
                      << Fixed(phiNorm) << std::setw(11) << Fixed(pir) << std::setw(6)
                      << r.maxChannels.size() << "  " << maxch
                      << (variant.empty() ? "" : "  [" + variant + "]") << "\n";

                const std::string key = std::string("routing=") + AlgoName(algo) +
                                        (variant.empty() ? "" : ",variant=" + variant) +
                                        ",traffic=" + PatternName(pattern) +
                                        ",method=" + MethodName(method) + ",mesh=" + meshTag;
                results << "RESULT,mode=phi," << key << ",flows=" << flows.size()
                        << ",phi=" << Fixed(r.phi) << ",phi_norm=" << Fixed(phiNorm)
                        << ",pir_max=" << Fixed(pir) << ",nmax=" << r.maxChannels.size()
                        << ",maxch=" << maxch << "\n";
                if (opt.channels)
                {
                    for (int ch = 0; ch < mesh.NumChannels(); ++ch)
                    {
                        results << "CHAN," << key << ",ch=" << mesh.ChannelName(ch)
                                << ",rho=" << Fixed(rho[ch]) << "\n";
                    }
                }
            }
        }
    }
    os << table.str() << results.str();
}

} // namespace noc
