interface MetricCardProps {
  value: string;
  label: string;
  detail?: string;
}

export function MetricCard({ value, label, detail }: MetricCardProps) {
  return (
    <div className="text-center p-6">
      <div className="text-3xl md:text-4xl font-bold text-text-primary font-mono mb-2">
        {value}
      </div>
      <div className="text-sm font-medium text-text-secondary mb-1">{label}</div>
      {detail && (
        <div className="text-xs text-text-muted">{detail}</div>
      )}
    </div>
  );
}
