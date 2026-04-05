interface StatusBadgeProps {
  status: 'Active' | 'In Review' | 'On Request' | 'Launching Soon';
}

export function StatusBadge({ status }: StatusBadgeProps) {
  const styles: Record<string, string> = {
    'Active': 'bg-accent-secondary/10 text-accent-secondary border-accent-secondary/20',
    'In Review': 'bg-accent-amber/10 text-accent-amber border-accent-amber/20',
    'On Request': 'bg-text-muted/10 text-text-muted border-text-muted/20',
    'Launching Soon': 'bg-accent-primary/10 text-accent-primary border-accent-primary/20',
  };

  return (
    <span className={`inline-flex items-center px-2.5 py-0.5 text-xs font-mono border rounded-sm ${styles[status] || styles['Active']}`}>
      {status}
    </span>
  );
}
